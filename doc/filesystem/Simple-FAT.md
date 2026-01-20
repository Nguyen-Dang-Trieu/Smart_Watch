# A Simple Filesystem

## Cấu trúc cơ bản
Ở mức cơ bản nhất, mỗi file được tổ chức dưới dạng danh sách liên kết các block. Block là đơn vị nhỏ nhất mà hệ thống có thể thực hiện thao tác read/write.

Kích thước block được tính theo byte, phổ biến là: 512B, 1KB, 2KB, ...  
Trong tài liệu S-FAT, thuật ngữ *sector* sẽ được dùng để chỉ các block có khả năng read/write.

Ví dụ: Một file "foo.txt" có kích thước là 1400 bytes được lưu trữ trong một filesystem có kích thước mỗi sector = 512 bytes. Điều gì sẽ xảy ra ?

<p align="center">
  <img src="./images/basic_sector.png" width="350">
</p>

- 2 sector đầu tiên sẽ đầy: 512 * 2 = 1024 bytes.
- Sector thứ 3 sẽ lưu trữ 376 byts, và còn lại 136 bytes không sử dụng.

Hệ thống Filesystem của tôi sẽ dựa trên FAT (File Allocation Table). Và tôi đặt tên cho dự án filesystem này là 
Simple-FAT.

Các thành phần cơ bản của Simple-FAT.
- `superblock`: Quản lí toàn bộ thông tin về FAT như sector, size sector, cluster, ...
- `directory area`: Nơi lưu trữ thông tin của file đã được tạo.
- `file allocation table`: Theo dõi các sector nào đã được sử dụng, sector nào free.
- `data area`: Dữ liệu thực sự của file được lưu trữ ở đây.

<p align="center">
  <img src="./images/cau_truc_sfat.png" width="350">
</p>

Tiếp tục với ví dụ bên trên, lúc này các thành phần của SFAT sẽ chứa.

Directory entry
<p align="center">
  <img src="./images/directory_entry.png" width="350">
</p>
- **filename**: tên file
- **size**: kích thước hiện tại file.
- **FAT-entry**: Chỉ số sector đầu tiên của file trong bảng FAT

File Allocation Table
<p align="center">
  <img src="./images/fat_1.png" width="350">
</p>

<p align="center">
  <img src="./images/fat_2.png" width="350">
</p>

Data blocks
<p align="center">
  <img src="./images/data_blocks.png" width="350">
</p>

## Thiết kế
### Superblock
Superblock là vùng đầu tiên của filesystem, dùng để mô tả toàn bộ layout của filesystem. Mọi thông tin cần thiết để mount và truy cập filesystem đều được lưu trữ tại đây.

Superblock có kích thước là 44 bytes. Với kích thước như này đủ để quản lí tất cả các thông tin trong filesystem bất kể có bao nhiêu bytes đi chăng nữa.

~~~c
typedef struct SuperBlock
{
  uint16_t total_sectors;         // Total number of sectors
  uint16_t sectors_per_cluster;   // Number of sectors in each block
  uint16_t bytes_per_sector;      // Number of bytes in each sector
  uint16_t available_sectors;     // Number of sectors available
  uint16_t total_direntries;      // Total number of directory entries (files)
  uint16_t available_direntries;  // Number of available dir entries
  uint8_t  fs_type;               // File system type (FA for SFAT)
  uint8_t  reserved[11];          // Reserved, all set to 0
  uint8_t  label[20];             // Not guaranteed to be NUL-terminated
}SuperBlock_t;
~~~

<p align="center">
  <img src="./images/superblock_struct.png" width="350">
</p>
Tổng bytes cho vùng superblock: 44 bytes -> Chiếm 1 sector.

Và ở đây, bạn có thể thắc mắc là cluster là gì?
> Trong filesystem, cluster là đơn vị cấp phát (allocation unit) – tức là đơn vị nhỏ nhất mà hệ thống dùng để cấp phát dung lượng cho file.

Trong S-FAT, 1 cluster = 1 sector.

### Directory area
Chứa các directory entry, mỗi entry lưu trữ metadata của một file trong hệ thống.

~~~c
typedef struct __attribute__((packed))
{
  uint8_t  name[10];     // Not guaranteed to be NUL-terminated
  int16_t  fat_entry;    // First data block
  uint32_t size;         // Size of the file in bytes
} DirEntry_t;
~~~

<p align="center">
  <img src="./images/directory_struct.png" width="350">
</p>

Số lượng tối đa file tối đa của SFAT là: 32 file.
-> Tổng bytes cần thiết cho vùng này: 32 * 16 = 512 bytes -> Chiếm 1 sector.

### File Allocation Table và Data Blocks
- Mỗi fat entry: có kích thước là 2 bytes.
- Số sector Data Area cần quản lý: 4096 - 1 - 1 - 16 = 4078 sector  
→ Cần 4078 FAT entry.

Dung lượng FAT:
4078 × 2 = 8156 bytes < 8192 bytes (16 sector) => FAT sẽ chiếm 16 sectors. 

Sơ đồ bên dưới thể hiện mỗi quan hệ giữa một fat entry với một sector của data blocks.
<p align="center">
  <img src="./images/fat_datablock.png" width="350">
</p>

Để hiểu rõ hơn, hãy xem ví dụ sau: tạo một file `foo.txt` có kích thước 1400 bytes. Hình ảnh dưới đây minh họa cách nội dung file này được biểu diễn trong FAT và Data Block.
<p align="center">
  <img src="./images/Ex_fat_datablock.png" width="350">
</p>

- Giá trị 0 trong FAT entry biểu thị data block chưa được sử dụng (FREE).
- Giá trị 0xFFFF biểu thị kết thúc chuỗi cấp phát của file (EOF).
- Các giá trị khác biểu thị chỉ số fat entry tiếp theo trong chuỗi liên kết của file.





Tham khảo:
- https://azrael.digipen.edu/~mmead/www/Courses/CS180/Simple-FS.html
- https://azrael.digipen.edu/~mmead/www/