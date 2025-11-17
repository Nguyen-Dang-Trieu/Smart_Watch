# ⌚ Smart Watch
## I. Giới thiệu
Dự án **Smart Watch** này được xây dựng từ đầu, bao gồm cả **hardware** và **software**, sử dụng **FreeRTOS** để chạy trực tiếp trên **2 core**.

> 📌 Note:  
> Trong giai đoạn đầu, phần cứng sử dụng các module sẵn có để tập trung phát triển phần mềm và ứng dụng. Sau khi phần mềm ổn định, phần cứng sẽ được tối ưu để phù hợp đúng với một smart watch.


## II. Kiến trúc hệ thống
Để học sâu về **STM32** và **ESP32**, thay vì dùng CPU đa lõi, dự án kết hợp **hai vi điều khiển** để tạo hệ thống “đa lõi”:
- **STM32F103C8T6**: quản lý giao tiếp với cảm biến và môi trường bên ngoài.  
- **ESP32**: xử lý giao diện người dùng (UI) và kết nối IoT.

### Cơ chế giao tiếp STM32 ↔ ESP32
Giống như CPU đa lõi cần cơ chế truyền thông nội bộ, ở đây **STM32** và **ESP32** trao đổi dữ liệu. Để chuẩn hóa định dạng gói tin, tôi đã triển khai một thư viện gọi là **"InCore"**. **InCore** được lấy cảm hứng từ thư viện [SerialTransfer](https://github.com/PowerBroker2/SerialTransfer).

Những tính năng của thư viện "InCore":
- **Packet structure**:
~~~
  Cấu trúc của Packet:
  [Start byte] [Packet ID] [payload length byte] [Payload bytes ... ] [8-bit CRC] [Stop byte].

  Trong đó:
  - Start byte: 1 byte có giá trị cố định là 0x7E
  - Packet ID: 1 byte định danh gói tin, mặc định là 0
  - payload length byte: 1 byte xác định số byte dữ liệu đã được mã hóa COBS trong packet
  - Payload bytes: Dữ liệu thực tế được truyền đi, tối đa 254 byte
  - 8-bit CRC: 1 byte kiểm tra lỗi dữ liệu
  - Stop byte: 1 byte có giá trị cố định là 0x81
~~~
- Kiểm tra lỗi bằng CRC8.
- Consistent Overhead Byte Stuffing: thuật toán để đóng gói dữ liệu mà không chứa byte `0x00`

## 📖 Mô tả
### 1. Hardware

| STT     |        Thiế bị           |  Vai trò |
| :-----: | :--------------------: | :--------------------: |
|    1    | STM32F103C8T6         | Thu nhập dữ liệu từ môi trường qua sensor & ngoại vi  |
|    2    | ESP32                 | Xử lí dữ liệu, UI & IoT |
|    3    | TTP226                | Touch input  | 
|    4    | DS18B20               |  Nhiệt độ |
|    5    | MAX30102              |  Nhịp tim & SpO2|
|    6    | MPU9250               |9-DOF IMU |
|    7    | BMP280                |Áp suất & độ cao | 

### 2. Driver Lib
Đây là thông tin về API để sử dụng với từng loại driver hiện có trong dự án.
| Thiết bị           | Trạng thái  | Kiểm thử   |  API     |
| :-------------------- | :----------------: | :-----: | :-----:  |
| TTP226                |       ✔️   |    ✔️   |  [Chi tiết](https://github.com/Nguyen-Dang-Trieu/Plant-water/blob/main/Doc/ATmega328p_API.md) |
| DS18B20               |       ✔️   |    ✔️   |  [Chi tiết](https://github.com/Nguyen-Dang-Trieu/Smart_Watch/blob/main/Doc/API/DS18B20_API.md)|        

## Hướng dẫn triển khai
### STM32 (Core 1)
- Mở Keil C → thư mục `cor1_stm`.
- Copy những driver cần thiết.
- Build và nạp firmware vào STM32.
  
### ESP32 (Core 2)
- Mở `VS Code` với `ESP-IDF` → thư mục `core2_esp`.
- Copy những driver cần thiết.
- Build và nạp firmware vào ESP32.

## 🔎Reference 
- Thư viện tham khảo cho MPU9250: https://github.com/DonovanZhu/9DoF_MARG_Madgwick_Filter/blob/master/Teensy/MPU9250/MPU9250_Madwick_Filter/MPU9250.h
- https://github.com/microsoft/IoT-For-Beginners/tree/main
- https://github.com/ZSWatch/ZSWatch
- https://github.com/fbiego
- Màn hình tròn, cảm ứng: https://www.proe.vn/1-28inch-touch-lcd
- Filesystem SPIFFS stm32 + freeRTOS: cần thiết kế thread safe. https://github.com/pellepl/spiffs + AT45DB161D-SU IC Nhớ Flash 16Mbit 8-SOIC (có thể dùng loại E để thay thế)
