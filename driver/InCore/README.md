Nguyên tắc hoạt động
COBS chia dữ liệu thành các khối (block). Mỗi khối bắt đầu bằng một byte code, cho biết từ byte đó có bao nhiêu
byte khác 0 cho đến trước byte 0 tiếp theo.

Ví dụ:
11 22 00 33 44
- Khối 1: 11 22 
- Khối 2: 33 44

-> Mã hóa
[03 11 22] [03 33 44] [00]

Lưu ý:
- Số lượng data chỉ có 254 byte + 1 byte code => max = 255 (0xFF)
=> Mỗi block có tối đa là 254 byte dữ liệu.