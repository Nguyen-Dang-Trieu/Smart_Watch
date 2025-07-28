# Smart Watch

## Tài liệu

- Kiến thức về STM32F103: https://www.notion.so/STM32F103x-217555493c6080afb24bca6f639818be

## 📌 Introduction

giới thiệu vê dự án smart watch

## 📖 Description

## 🛠️ Devices
### 1. Hardware

| STT     |        Name           | Price   |
| :-----: | :-------------------- | :------:|
|    1    | Raspberrby 3B+        |    💰   | 
|    2    | Arduino Uno           |    💰   | 
|    3    | DHT22                 |    💰   |  
|    4    | BH1750                |    💰   |   
|    5    | Soil moisture         |    💰   | 
|    6    | DC water pump motor   |    💰   | 
|    7    | Module Relay 5V       |    💰   |  

### 2. Driver Lib
| Device Name           | Library Completed  | Check   |  API     |
| :-------------------- | :----------------: | :-----: | :-----:  |
| TTP226                |       ✔️          |    ✔️   |  [Detail](https://github.com/Nguyen-Dang-Trieu/Plant-water/blob/main/Doc/ATmega328p_API.md) |
| DS18B20               |       ✔️          |    ✔️   |  [Detail](https://github.com/Nguyen-Dang-Trieu/Smart_Watch/blob/main/Doc/API/DS18B20_API.md)|        

### 3. Software Lib
| OneWire               |       ✔️          |    ✔️   |          |


| Device Name      | Library Completed  | Check   | 
| :--------------- | :----------------: | :-----: | 
| STM32F103C8T6    |       ✔️          | ✔️      | 
| ESP32            |       ❌          | ❌      |
| TTP226           |       ✔️          | ✔️      | 
| DS18B20 (One-Wire)|       ✔️          | ✔️      |
| MAX30100         |       ❌          | ❌      |
| MPU6050          |       ❌          | ❌      |
| LCD ST7789       |       ❌          | ❌      |
| SD CARD (SPI)    |       ❌          | ❌      |
| JSON             |       ❌          | ❌      |
| FreeRTOS         |       ✔️          | ❌      |

gửi data dạng JSON, STM32F103C8T6 gửi dữ liệu qua ESP32 thông qua UART kết hợp DMA

Tìm hiểu cách viết thư viện JSON: https://github.com/DaveGamble/cJSON và ArduinoJson

https://github.com/microsoft/IoT-For-Beginners/tree/main

giao diện dồng hồ với RTC: https://www.youtube.com/watch?v=35Z0enhEYqM
