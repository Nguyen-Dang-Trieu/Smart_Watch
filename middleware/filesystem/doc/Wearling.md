## Cân bằng hao mòn

Có 2 loại wear leveling
- Wear-leveling dynamic
+ Chỉ xoay vòng vùng hay ghi
+ Những vùng "only read" hay "ít ghi" không tham gia

=> Ví dụ: vùng để lưu trữ logdata.

- Wear-leveling static
+ Không có block nào được "ngồi yên mãi mãi"
+ Các block sẽ luân phiên nhau di chuyển, kể cả "data cố định" cũng sẽ di chuyển sang block khác.

Yêu cầu: tốn nhiều tài nguyên của CPU hơn dynamic.

## Kéo dài tuổi thọ của EEPROM
EEPROM chết theo cell yếu nhất.
~~~
Block A: 1.000.000 writes ❌
Block B: 50.000 writes
Block C: 20.000 writes
~~~
→ thiết bị vẫn hỏng, dù phần lớn EEPROM còn mới.

Wear leveling cố gắng đạt được
~~~
A: 350.000
B: 340.000
C: 360.000
~~~

## Nghich lý
Nếu ta lưu meta data (chẳng hạn như superblock của SFAT, ...) vào trong EEPROM thì chính meta data lại là thứ thay đổi nhiều nhất mỗi khi hệ thống chạy

giải pháp được đề xuất: Lưu thông tin này trên MCU
- RAM:
    + Không bị wear
    + Đọc/ghi vô hạn vs tốc độ nhanh

Nhưng RAM sẽ mất hết dữ liệu khi hệ thống off 😱
-> Điều này có nghĩa là những dự liệu về superblock, hay cả lịch sử hao mòn của từng block phần cứng đều sẽ bị xóa sạch

Giải pháp: Thiết kế mạch phát hiện mất nguồn
=> Khi sắp hết nguồn điện thì metadata từ RAM -> EEPROM

## Phương pháp tiếp cận software để giảm wear-leveling
- Tạo ra một thuật toán hướng lần ghi tiếp theo vào số khối có lần ghi ít nhất.
- Các block có data cố định phải được di chuyển
- Số chu kỳ của mỗi block sẽ được theo dỗi bằng bộ đếm.



## Ghi chú thiết kế FTL
- Đại loại tôi hiểu là khi ta sửa một dữ liệu gì đó nó sẽ ghi vào page mới thay vì page cũ để tránh hao mòn flash
- Vì dữ liệu cứ nhảy lung tung như vậy nên ta cần một mapping table để quản lí
 + Địa chỉ logic LBA: Địa chỉ mà ứng dụng nhìn thấy.
 + Địa chỉ vật lí PBA: Địa chỉ thực tế nằm trên phần cứng

- Maptable này là một metadata được truy cập thường xuyên nên nằm trên RAM

Như trong hình minh họa thì
- Phía bên trái là RAM chứa mapptable, bên phải là bộ nhớ flash (phần cứng).
- 300 là địa chỉ logic được ánh xạ tới địa chỉ vật lí là X trên flash

### Vấn đề
- Map table rất lớn
Ví dụ nếu một flash có dung lượng là 2 TB -> RAM cần 2TB 

Từ đó ta có thể thấy không thể dùng lượng RAM lớn như vậy

+ Phần lớn mapptable sẽ được lưu trên chip flash, chỉ một phần nhỏ sẽ được lưu trên RAM để có thể thao tác


Tra RAM: Để tìm trang logic 300, FTL nhìn vào bảng trong RAM. Nó thấy trang 300 thuộc dải (range) 256 - 383.
Tìm vị trí bảng mapping: Bảng trong RAM chỉ ra rằng chi tiết ánh xạ cho dải này đang nằm ở flash page 47.
Đọc Flash lần 1: SSD phải đọc Mapping page 47 từ chip Flash lên để biết trang 300 nằm ở đâu.
Đọc Flash lần 2: Sau khi có thông tin từ bước 3, SSD mới thực sự đi tới Data page 300 để lấy dữ liệu cho bạn.


Tại sao lại nói là "trả giá bằng hiệu năng"?
 - Tốn thời gian gấp đôi: Thay vì chỉ cần 1 lần truy cập Flash (như hình 1), bây giờ bạn tốn ít nhất 2 lần đọc Flash để lấy được 1 mẩu dữ liệu.
 - Tốc độ chậm lại: Chip Flash chậm hơn RAM rất nhiều, nên việc phải đi "tra cứu" trên Flash làm giảm tốc độ phản hồi của SSD.


 Và để giảm chi phí đọc dữ liệu khi ánh xạ, ta thêm "cache"


 Mỗi lần update một page mà ta cứ truy cập map table liên tục rồi sửa đổi liên tục thì sẽ ảnh hướng đến hiệu suất rất xấu. Thay vào đó chỉ cần đánh dấu dữ liệu đã được update và đợi khi nhiều nhiều thì mới chỉnh sửa luôn một lần.

 FTL cũng duy trì một bitmap 
 - Theo dõi các pages nào không hợp lệ
 - Để tiện cho việc thu gom rác (GC)

 - PVB (Page Validity Bitmap) - nằm trên RAM.
 + Cho phép lựa chọn đối tượng cần xử lí trong quá trình thu gom rác.
 + Chọn những page cần di chuyển

 RAM là một loại bộ nhớ sẽ mất dữ liệu khi tắt điện.
 
 vậy làm thế nào để có thể khôi phục PVB
 -> Trả lời: quét tất cả trang ánh xạ

 Nhưng lại có một vấn đề khác xảy ra
 - Kích thước của PVB tỷ lệ thuận vs dung lượng, khi flash mở rộng lên đến TB thì kích thước PVB rất lớn
 => Dẫn đến tốn rất nhiều thời gian để khôi phục lại PVB

 giải pháp đơn giản:
 - Lưu trữ PVB trên flash nhưng sẽ ảnh hưởng đến hiệu năng

 Mỗi lần logical update thì ta cũng phải update PVB -> write 2 lần

 Thao tác ghi chậm hơn so với đọc, mặc khác nó cũng làm hao mòn flash
Do đó giải pháp lưu trữ PVB trên flash
- Làm giảm bằng thông
- Hao mòn memory

Vấn đề
Write-amplification là số lượng thao tác ghi vật lí bên trong mỗi thao tác ghi logic

Mục tiêu:
- Lưu trữ PVB trên flash
- giữ Write-amplificatio ở mức thấp nhất.
- Tra cứu bảng nhanh
