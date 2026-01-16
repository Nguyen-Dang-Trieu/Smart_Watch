PVB is updated once per application write
Ý nghĩa: Mỗi khi ứng dụng ghi dữ liệu mới vào Flash, hệ thống phải cập nhật PVB ngay lập tức.

Tại sao: Khi bạn ghi đè dữ liệu mới, dữ liệu ở vị trí cũ sẽ trở thành "rác" (invalid). Hệ thống cần đánh dấu vào PVB để biết rằng: "Trang cũ này bây giờ là rác rồi, đừng đọc nó nữa".

Đặc điểm: Tần suất này xảy ra rất thường xuyên (mỗi lần ghi là một lần cập nhật).

2. PVB is accessed once per garbage-collection (GC)
Ý nghĩa: Hệ thống chỉ thực sự cần "đọc" kỹ bảng PVB khi nó thực hiện quá trình Dọn rác (Garbage Collection).

Tại sao: Khi GC chạy, nó cần quét qua các Block để xem trang nào còn dùng được (Valid) thì giữ lại, trang nào là rác (Invalid) thì xóa đi để giải phóng bộ không gian. PVB chính là "bản đồ" để GC biết cái nào cần giữ, cái nào cần vứt.

3. GC happens once for every ≈ 100 application writes
Ý nghĩa: Cứ khoảng 100 lần ứng dụng ghi dữ liệu thì hệ thống mới cần dọn rác 1 lần.

Con số này nói lên điều gì?

Tần suất ghi/cập nhật PVB là 100 đơn vị.

Tần suất đọc/truy cập PVB để dọn rác chỉ là 1 đơn vị.

Tóm lại lại quy trình thực hiện 
- Ghi vào nháp (RAM): Nhanh, tạm thời.
- Chép vào sổ cái (Flash): Khi nháp đầy, chép một mớ vào sổ để lưu trữ lâu dài.
- Sắp xếp lại sổ (Compaction): Khi sổ quá lộn xộn, dành thời gian sắp xếp lại cho dễ tra cứu.

Khi một page bị đánh dấu là "invalidated", tạo entry
- Ghi lại ID của block chứa page bị hỏng đó.
 + Ta sẽ thực hiện việc ghi xóa trên block, ta cần phải đánh dấu block nào có nhiều page rác nhất để ưu tiên dọn dẹp trước
- Tạo ra một dãy bitmap có đúng số lượng page trong block. Ban đầu các bit được đặt là 0 để đánh dấu page còn hoạt động, 1 - đã hỏng

Khi một block được "xóa", tạo mục nhập entry
- ID của block bị xóa
- Tất cả các bitmap thì set = 0
- Bit cuối erase flag = 1

=> Thêm nó vào trong buffer


Cuối cùng sắp xếp theo thứ tự và push qua cho flash nhưng ở level 0
- Khi có tiếp những buffer tiếp theo sẽ tiến hành sắp xếp lại tiếp và xử lí các entry bị trùng.

Erase Flag: Là một cái "cờ" báo hiệu rằng toàn bộ Block đó đã được xóa sạch (Erase).

Quy tắc: Nếu một bản ghi mới hơn (More recent) báo rằng "Block này đã bị xóa rồi", thì tất cả các bản ghi cũ hơn (Less recent) về việc "trang này hỏng, trang kia hỏng" trong Block đó đều vô nghĩa.

Tại sao? Vì khi Block đã bị xóa sạch, mọi trang trong đó đều trở về trạng thái trống (Valid/Empty). Những ghi chép cũ về việc trang nào là rác không còn đúng nữa.