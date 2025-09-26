# Smart Watch

## Tài liệu

- Kiến thức về STM32F103: https://www.notion.so/STM32F103x-217555493c6080afb24bca6f639818be

## 📌 Introduction

giới thiệu vê dự án smart watch

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
