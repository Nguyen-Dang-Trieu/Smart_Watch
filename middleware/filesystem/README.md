Nếu dùng phần cứng là AT45DB161D-SU IC Nhớ Flash 16Mbit 8-SOIC
Thông số:
- 512Bytes x 4046 pages


sector là đơn vị nhỏ nhất mà filesystem đọc/ghi.
- total_sector = 4096

byte_per_sector là số byte trong một sector.
- byte_per_sector = 512 bytes

sectors_per_cluster là số lượng sector trong mỗi block
-> Khi cấp phát, filesystem sẽ cấp phát theo đơn vị cluster chứ không phải theo sector.

Vd:
- sectors_per_cluster = 1
+ một cluster = 1 secotr = 512 bytes

Uư điểm: Tiết kiệm không gian
Nhược: FAT table lớn, nhiều entry hơn, tìm kiếm chậm hơn.

- sectors_per_cluster = 4
+ Một cluster = 4 sector = 4 x 512 = 2048 bytes

unsigned short available_sectors;    // Number of sectors available
- cho biết số lượng sector chưa sử dụng

unsigned short total_direntries;     // Total number of directory entries
Tổng số entry mà thư mục gốc có thể chứa
- Mỗi directory entry đại diện cho một file.
- Filesystem cố định entry và không tự động mở rộng.

  unsigned char fs_type;               // giúp xác định loại filesytem
  unsigned char reserved[11];          // Không dùng ngay dùng để dự phòng cho tương lai, tất cả đều là 0
  unsigned char label[8];              // Tên ổ đĩa để user/driver dễ nhận biết



Lưu ý:
- fat_entry sẽ chứa địa chỉ của block data/cluster đầu tiên nhưng điều đó là do bản thân mk nghĩ.
- Trong thực tế, fat_entry lưu index của block/sector.

Vd:
fat_entry = 0 -> cluster 0
fat_entry = 1 -> cluster 1
fat_entry = 2 -> cluster 2

Sau đó filesystem sẽ tự tính offset:
address = data_area_start + cluster_index + cluster_size.

## Xử lí file tồn tại đã ghi trước đó
mode = 'w' (ghi đè)
- xóa cluster cũ
- offset = 0
- ghi dữ liệu mới -> cấp cluster mới nếu cần
- update size

mode = 'a' (append)
offset = size
ghi vào cluster cuối cùng → nếu đầy thì cấp cluster mới
update size

mode = "r+" (modifi)
tìm cluster theo offset
ghi đè dữ liệu từ vị trí đó
nếu vượt size → mở rộng → cấp cluster
update size nếu vượt

## ⚠️ Điểm chú ý cần cải thiện
1. Quản lí con trỏ khi mount/unmount
2. Kiểm tra lỗi chưa đầy đủ -> Nên có hệ thống ERR rõ ràng

## Phải thiết kế theo nguyên tắc Collect → verify → commit
- collect: tìm kiếm cluster free đủ số lượng cần dùng, lưu tạm vào ram(list, linked list, array, ..)
- verify: Kiểm tra mọi điều kiện
  + có đủ cluster không
  + cluster cuối có phải là SFAT_END_CLUSTER ?
  + start_entry_add có hợp lệ
- commit: Tại thời điểm này thì mới nên cập nhập cluster

## Để thiết kế được tính năng wear leveling thì cần tìm hiểu về log-structured file systems (tùy)
https://zhuanlan.zhihu.com/p/643911944
https://github.com/qmk/qmk_firmware/tree/master/quantum/wear_leveling
h


## Thiết kế cache 
http://daslab.seas.harvard.edu/classes/cs265/files/visiting-lectures/NivDayan-DesignFTL.pdf
Dhara


## Mục tiêu hiện tại 
- Data page: write-through
- Mapping page: write-back, dirty đại diện cho mapping metadata chưa được ghi xuống flash


dirty = true
⇔ mapping table trong RAM ≠ mapping table trên flash



- Mặc định khi khởi động hệ thống thì toàn bộ bitMap trong block đều là 0 hết

Các ý tưởng thiết kế:
- Trong FTL, sẽ duy trì một bảng bitMap toàn bộ hệ thống.
 + bitMap: những phần tử block trong này đã từng được sử dụng. Nếu chưa từng sử dụng or đã bị gc dọn thì sẽ không 
 còn trong bảng bitMap này nữa


### Những điểm kiến thức mới
- Copy on write
- Write-back

### Nguyên tắc vàng khi dùng biến code C
- Definition (cấp bộ nhớ) ở file .c (1 nơi duy nhất)
- Declaration (extern) ở file .h
- Header -> Không bao giờ được định nghĩa biến

Câu hỏi: Khi nào truyền 1 thuộc tính của struct hay toàn bộ struct cho một hàm ?

- Khi hàm đó sử dụng nhiều thuộc tính của struct -> Truyền toàn bộ struct
- Khi hàm đó chỉ sử dụng 1 thuộc tính của struct -> Truyền chính xác thuộc tính đó.


Để tránh include vòng

thì trong file.h ta chỉ cần 
~~~
/* ✅ Forward declaration */
typedef struct Flash_Translation_Layer FTL_t;
~~~

Trong file.c thì 
~~~
#include "FTL_map.h"
~~~


Làm một ví dụ project nhỏ để luyện ép kiểu truyền địa chỉ
spare_t spare;  // spare của block hiện tại
spare.fields.logical_page = logical_page;
AT45DB_WritePage(physical_page, buffer, (uint8_t *)&spare);