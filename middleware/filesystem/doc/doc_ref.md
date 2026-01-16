Theo như bài đọc tham khảo
- Có 8191 sector, kích thước của mối sector = 512 bytes => Tổng dung lượng 4MB: superblock, FAT, directory và vùng
dữ liệu
- Cluster đơn vị phân bổ cho file có kích thước =  1 sector.
- Root directory có 1024 file (file = directory entry)

directory area = vùng lưu trữ cố định trong filesystem dùng để quản lí danh sách các file
- Không chứa dữ liệu cùa file.
- Chứa metadata của file


Directory entry = một record trong directory area

Nói nôm na dễ hiêu là với directory area = 16KB thì có tổng cộng là 1024 file + kích thước mỗi file là 16 bytes.

# FAT = 16KB
Filesystem có 8192 sector, mỗi sector có 512 bytes

FAT lưu một bảng , mỗi sector ứng với một entry trong FAT
Mỗi FAT entry là kiểu unsigned short (2 bytes)
~~~
8192 sectors × 2 bytes = 16384 bytes = 16 KB
~~~

Ý nghĩa:
- FAT[n] cho biết sector n tiếp theo nằm ở đâu
- nếu = 0xFFFF nghĩa là kết thúc file.
- nếu = 0 -> sector chưa dùng

Lưu ý:
- Danh sách liên kết thông thường, mỗi node sẽ chứa "data + next", next sẽ trỏ đến node tiếp theo
- Tuy nhiên, FAT tách dữ liệu khởi liên kết
    + Block chỉ chứa thuần dữ liệu, không chứa liên kết nào cả.

Thay vì lưu "next" trong block, FAT lưu ngoài dưới dạng bảng
~~~
FAT[0] = 5      // sector 0 tiếp theo -> sector 5
FAT[5] = 99     // sector 5 tiếp theo -> sector 99
FAT[99] = 0xFFFF // kết thúc file
~~~

Lưu ý:
Kích thước của supberblock luôn là 32 byte, mỗi directory entry là 16byte. 
- Những thứ khác nhau trong dự án này là:
 + total_sector
 + sector_per_cluster
 + byte_per_sector
 + total_direntires

 Về bản chất 
 - Kích thước của filesystem (datablock) phụ thuộc vào số lượng sector và kích thước của mỗi sector. Chỉ cần nhân 2 số này với nhau sẽ có số lượng byte có sẵn.
 - Số lượng file thì phụ thuộc vào directory entries.

 Lưu ý:
 Một số filesystem không đặt giới hạn về số lượng file bởi vì thư mục có thể phát triển thêm. Trong dự án này, để giữ cho mọi thứ đơn giản, ta sẽ có một vùng thư mục cố định.


 - 0 là biểu thị một khối không sử dụng
 - FFFF đại diện cho phần cuối danh sách các khối

 (fat entry là con trỏ của khối đầu tiên)