# Smart Watch

## Tài liệu

- Kiến thức về STM32F103: https://www.notion.so/STM32F103x-217555493c6080afb24bca6f639818be

## 📌 Introduction

giới thiệu vê dự án smart watch

## 📖 Description

## 🛠️ Driver library

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
