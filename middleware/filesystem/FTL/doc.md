## SFAT Layer
Thứ mà SFAT nhìn thấy là logical page chạy từ 0 -> N

## FTL Layer


- FTL ghi tuần tự các page trong cùng một block
- Recall erase-before-write rule
 + Copy dữ liệu còn dùng sang block khác
 + Xóa block cũ
 + Ghi lại dữ liệu mới

- Giảm chi phí bằng cách "Out-of-place update"

Thế "Out-of-place update" là gì?
- Không ghi đè dữ liệu cũ
- Ghi dữ liệu sang page khác
- Đánh dấu dữ liệu cũ là invalid

📌 Ví dụ:

*Lúc đầu: 

LBA 10 → Block 2, Page 1

* Cập nhập:
~~~
Ghi dữ liệu mới vào Block 5, Page 0
Cập nhật mapping:
LBA 10 → Block 5, Page 0
~~~

Page cũ: Block 2, Page 1 → INVALID

## Slides 13
FTL cần phải duy trì một bảng mapping table từ logical -> physical

table mapping rất lớn 

-> giải pháp: lưu trữ trong flash

Trong slide này, app đang muốn đọc logical page 300 -> FTL phải tìm ra nó dang ở trên physical page nào.
Tiết kiệm RAM nhưng đánh đổi tốc độ (phải đọc flash để tra mapping)

Ý tưởng thiết kế
- Mapping không theo từng page mà theo tửng range

Chia logical page thành các "range"
~~~         
                                    Flash page
Range 0   : logical 0   – 127   ->   16
Range 1   : logical 128 – 255   ->   94
Range 2   : logical 256 – 383   ->   47
Range 3   : logical 384 – 511   ->   60
~~~
📌 Mỗi range = 128 logical page

Ưng dụng muốn đọc logical page là 300 -> nằm trong range 2 

Và dữ liệu mapping table (logical -> physical) sẽ chứa trong mapping page là 47 (phần cứng flash nhé)

Ví dụ:
~~~
Entry 0  → logical 256 → physical 900
Entry 1  → logical 257 → physical 901
Entry 2  → logical 258 → physical 902
...
Entry 44 → logical 300 → physical 812
...
Entry 127→ logical 383 → physical 950
~~~

Tại sao gọi là mapping page?
Page này: 
- Không chứa data user.
- Khong chứa code
- Chứa logical -> physical


Tại sao lại là “page 47” mà không phải chỗ khác?
- Mapping page cũng nằm trong flash
- Nó được FTL cấp phát như page bình thường

Có thể: move, copy, wear-leveling, GC
👉 Số 47 chỉ là ví dụ


Giờ tôi sẽ diễn giải lại từng bước
- Bước 1: Tra RAM -> mapping page = 47
- Bước 2: Đọc flash page = 47 -> mapping[300] = 812
- Bước 3: Đọc data thật, đọc flash page = 812 -> Trả về cho app.

=> Kết luận quan trọng: 📌 Đọc 1 page = 2 lần đọc flash

Để giảm thiểu chi phí mỗi lần đọc dữ liệu => Thêm cache

Cache
- Chưa ghi xuống flash vội.
- Đánh dấu là "dirty"

Khi app update logical page 300
Bước 1: Ghi data
- FTL chọn physical page mới Z
- ghi data page Z

Bước 2: Không đụng mapping page 47
Bước 3: update cache
~~~c
cache[300] = Z
cache[300].dirty = true
~~~

Khi nào mới update xuống flash
- Cache đầy.
- Chuẩn bị flush

## Slide 20
FTL có một bit map có nhiệm vụ
- Theo dõi page nào có giá trị là invalid
- Cần thiết để dọn rác
- Cho phép lựa chọn đối tượng trong quá trình thu gôm rác
- Chọn những page cần di chuyển


## Slide Tổng kết
- Logical pages are updated "out-of-place" 
- Không ghi đè lên dữ liệu cũ
- Khi dữ liệu cần cập nhập:
 + Không ghi lên trang cũ
 + Ghi sang trang physical page mới

 📌 Ví dụ:
 ~~~
 Logical page 5
Lần 1 → Physical page 100
Cập nhật → Physical page 350
 ~~~

 Garbage-collection takes place to free space  
  GC được dùng để giải phóng bộ nhớ
  - Sau nhiều lần update: Có rất nhiều page invalid
  Công việc của GC
  - Tìm block có nhiều page là invalid
  - Copy các trang còn "live" sang block khác
  - Erase block cũ để dùng lại
📌 GC là lý do flash bị chậm đột ngột khi đang chạy

- Bảng mapping dùng để quản lí việc logical page -> physical page
- Bảng mapping được lưu trữ trên flash

- PVB theo dỗi page nào sống trong GC
- PVB dược lưu trên RAM do thay đổi thường xuyên


Bây giờ, giải thích lí do tại sao phải ghi ít nhất là 2 lần khi dùng PP đơn giản là lưu PVB trên flash
- Dữ liệu update theo kiểu "out-of-places": Mỗi lần ghi là page vật lí khác nhau

~~~
logical page X -> page 1

update
logical page X -> page 2
~~~

Lúc này sẽ có 2 trạng thái đổi page
~~~
page 1 từ valid -> invalid
page 2 chuyển thành valid
~~~
Điều này sẽ dẫn đến nếu ta muốn cập nhập trạng thái page trong PVB thì cần phải write 2 lần
- Tốn hiệu suất
- Gây wear cho memory

giải pháp
PVB is updated once per application write 

👉 Mỗi lần ứng dụng ghi dữ liệu → PVB phải cập nhật
Logical write →
- 1 page mới valid
- 1 page cũ invalid

Nhưng về mặt logic: Đây được tính là 1 lần update PVB / application write

PVB is accessed once per garbage-collection operation
-> PVB chỉ được đọc thật sự khi GC chạy

Tần suất của GC thấp hơn rất nhiều so với logic write
VD: 100 lần app write ~ 1 lần GC



# Slide 78
- Tìm kiếm tất cả các thông in của block cần GC từ tầng mới nhất -> cũ nhất
 + Từ Ram - memtable -> L0 -> L1

- Điều kiện dừng
 + Nếu gặp block ID có erase_flag = 1, dừng, không tiếp tục đọc các tầng cũ hơn vì khi erase_flag = 1 nghĩa là đã reset vòng đời của block
 + Nếu chưa gặp block ID có erase_flag = 1 -> Tiếp tục search cho tiếp tầng cuối trong lsm-tree.

- Sau đó, ta sẽ merge các lịch sử thay đổi các bitMap của block ID đó
 + Lưu ý:Chỉ những state của block ID nằm sau state block ID khi erase_flag = 1

 => Lúc này ta đã có state của block và có thể tiến hành GC


 ## Question
 1 block có 8 page, vậy câu hỏi đặt ra thì block đó phải chứa bao nhiêu invalid page thì mới tiến hành GC

 Lấy một ví dụ về việc GC sớm khi trong block còn nhiều page valid.
 ~~~
 Block: 7 valid, 1 invalid

Muốn ghi 1 page mới → GC
- copy 7 page valid sang block khác
- erase block cũ
- ghi 1 page mới
 ~~~
 → Tốn 8 page write thực tế cho 1 page dữ liệu → write amplification = 8

 Khi GC muộn (block còn ít page valid, nhiều invalid):
 ~~~
 Block: 3 valid, 5 invalid
Muốn ghi 1 page mới → GC
- copy 3 page valid sang block khác
- erase block cũ
- ghi 1 page mới
 ~~~
 → Tốn 4 page write thực tế cho 1 page dữ liệu → write amplification = 4 (giảm WA)

 ✅ Kết luận: GC khi block còn ít page valid sẽ giảm write amplification, vì phải copy ít data valid.

 Note: 1 page có 3 trạng thái: free, valid, invalid