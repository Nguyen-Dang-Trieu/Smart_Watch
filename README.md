# Smart Watch
## Introduction
Dự án Smart Watch này được xây dựng từ đầu, cả hardware và software sử dụng hệ điều hành freeRTOS để chạy trực tiếp trên 2 core.

📌Note:   
Nếu nhìn vào dự án này, bạn có thể suy nghĩ là những phần cứng tôi sử dụng hiện tại thì khá là "lớn" so với một chiếc smart watch. Tuy nhiên ở đây, trong giai đoạn đầu của dự án, tôi chỉ muốn tập trung vào phát triển phần mềm và ứng dụng nên chỉ sử dụng những phần cứng có sẵn, dễ dàng tiếp cận ở trên thị
trường để có thể hoàn thiện những tính năng cơ bản mong muốn trước. Sau khi phần mềm đã ổn định, thì tôi sẽ quan tâm và thiết kế lại phần cứng cho phù hợp hơn với một chiếc smart watch.


### Mục đích, ý tưởng thiết kế
Để học hỏi sâu hơn về stm32 và esp32, thay vì sử dụng một CPU đa lõi tích hợp sẵn, tôi kết hợp **hai vi điều khiển này** để tạo thành hệ thống “đa lõi”:
- **STM32F103C8T6**: chịu trách nhiệm giao tiếp với các cảm biến và môi trường bên ngoài.
- **ESP32**: xử lý phần giao diện người dùng (UI) và kết nối IoT.

#### Giao tiếp giữa STM32 và ESP32
Giống như CPU đa lõi cần cơ chế truyền thông nội bộ, ở đây **STM32** và **ESP32** trao đổi dữ liệu. Để chuẩn hóa định dạng gói tin, tôi đã triển khai một thư viện gọi là "InCore".

Những tính năng của thư viện "InCore":
- Định dạng gói tin dưới dạng packet:
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
- Check error: CRC8
- Consistent Overhead Byte Stuffing: thuật toán để đóng gói dữ liệu mà không chứa byte `0x00`

## 📖 Description
### 1. Hardware

| STT     |        Name           | 
| :-----: | :-------------------- | 
|    1    | STM32F103C8T6         |   
|    2    | ESP32                 |  
|    3    | TTP226                |    
|    4    | DS18B20               |   
|    5    | MAX30102              |  
|    6    | MPU9250               |
|    7    | LCD ST7789            | 
|    7    | SD CARD (SPI)         |  
|    7    | BMP280                |  

### 2. Driver Lib
Đây là thông tin về API để sử dụng với từng loại driver hiện có trong dự án.
| Device Name           | Library Completed  | Check   |  API     |
| :-------------------- | :----------------: | :-----: | :-----:  |
| TTP226                |       ✔️          |    ✔️   |  [Detail](https://github.com/Nguyen-Dang-Trieu/Plant-water/blob/main/Doc/ATmega328p_API.md) |
| DS18B20               |       ✔️          |    ✔️   |  [Detail](https://github.com/Nguyen-Dang-Trieu/Smart_Watch/blob/main/Doc/API/DS18B20_API.md)|        




## 🔎Reference 
- Thư viện tham khảo cho MPU9250: https://github.com/DonovanZhu/9DoF_MARG_Madgwick_Filter/blob/master/Teensy/MPU9250/MPU9250_Madwick_Filter/MPU9250.h
- https://github.com/microsoft/IoT-For-Beginners/tree/main
- giao diện dồng hồ với RTC: https://www.youtube.com/watch?v=35Z0enhEYqM
- https://github.com/ZSWatch/ZSWatch
- https://github.com/fbiego
- Màn hình tròn, cảm ứng: https://www.proe.vn/1-28inch-touch-lcd
- Filesystem SPIFFS stm32 + freeRTOS: cần thiết kế thread safe. https://github.com/pellepl/spiffs + AT45DB161D-SU IC Nhớ Flash 16Mbit 8-SOIC (có thể dùng loại E để thay thế)
- https://github.com/PowerBroker2/SerialTransfer: thư viện để chuẩn hóa việc truyền dữ liệu của STM32 và ESP32
