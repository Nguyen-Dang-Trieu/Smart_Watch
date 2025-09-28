# Smart Watch

## Tài liệu

- Kiến thức về STM32F103: https://www.notion.so/STM32F103x-217555493c6080afb24bca6f639818be

## 📌 Introduction

Dự án Smart Watch này hiện vẫn đang trong quá trình phát triển nên chắc chắn còn nhiều hạn chế, mong bạn đọc thông cảm.

### Mục đích
Dự án được thực hiện nhằm:
- Nâng cao kỹ năng lập trình embedded.
- Tìm hiểu sâu hơn về vi điều khiển, hệ điều hành, giao tiếp và thiết kế hệ thống nhúng.
- Làm một sản phẩm mẫu phục vụ cho mục tiêu xin việc trong tương lai.

### Ý tưởng
Thay vì sử dụng một CPU đa lõi tích hợp sẵn, tôi kết hợp **hai vi điều khiển** để tạo thành hệ thống “đa lõi”:
- **STM32F103C8T6**: chịu trách nhiệm giao tiếp với các cảm biến và môi trường bên ngoài.
- **ESP32**: xử lý phần giao diện người dùng (UI) và kết nối IoT.

#### Giao tiếp giữa STM32 và ESP32
Giống như CPU đa lõi cần cơ chế truyền thông nội bộ, ở đây **STM32** và **ESP32** trao đổi dữ liệu qua **UART + DMA**.  
Để chuẩn hóa định dạng gói tin, tôi sử dụng và rework lại thư viện mã nguồn mở **SerialTransfer**, biến nó thành một phiên bản “customized” phù hợp hoàn toàn với yêu cầu của dự án.


## 📖 Description

## 🛠️ Devices
### 1. Hardware

| STT     |        Name           | 
| :-----: | :-------------------- | 
|    1    | STM32F103C8T6         |   
|    2    | ESP32                 |  
|    3    | TTP226                |    
|    4    | DS18B20               |   
|    5    | MAX30102              |  
|    6    | MPU6050               |
|    7    | LCD ST7789            | 
|    7    | SD CARD (SPI)         |  
|    7    | BMP280                |  

### 2. Driver Lib
| Device Name           | Library Completed  | Check   |  API     |
| :-------------------- | :----------------: | :-----: | :-----:  |
| TTP226                |       ✔️          |    ✔️   |  [Detail](https://github.com/Nguyen-Dang-Trieu/Plant-water/blob/main/Doc/ATmega328p_API.md) |
| DS18B20               |       ✔️          |    ✔️   |  [Detail](https://github.com/Nguyen-Dang-Trieu/Smart_Watch/blob/main/Doc/API/DS18B20_API.md)|        

### 3. Software Lib
| Name                  | Library Completed  | Check   |  API     |
| :-------------------- | :----------------: | :-----: | :-----:  |
| OneWire               |       ✔️          |    ✔️   |  Detail  |
| JSON                  |       ❌          |    ❌   |  Detail  |
| FreeRTOS              |       ✔️          |    ✔️   |     ⚠️     |
| OTA                   |       ❌          |    ❌   |     ⚠️     |


## 🔎Reference 
- gửi data dạng JSON, STM32F103C8T6 gửi dữ liệu qua ESP32 thông qua UART kết hợp DMA
- Tìm hiểu cách viết thư viện JSON: https://github.com/DaveGamble/cJSON và ArduinoJson
- https://github.com/microsoft/IoT-For-Beginners/tree/main
- giao diện dồng hồ với RTC: https://www.youtube.com/watch?v=35Z0enhEYqM
- https://github.com/ZSWatch/ZSWatch
- https://github.com/fbiego
- Màn hình tròn, cảm ứng: https://www.proe.vn/1-28inch-touch-lcd
- Filesystem SPIFFS stm32 + freeRTOS: cần thiết kế thread safe. https://github.com/pellepl/spiffs
- https://github.com/PowerBroker2/SerialTransfer: thư viện để chuẩn hóa việc truyền dữ liệu của STM32 và ESP32
