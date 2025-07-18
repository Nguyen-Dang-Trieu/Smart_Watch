# Smart Watch

## Tài liệu
- Ghi chú sử dụng thư viện LL trong stm32: https://viblo.asia/p/su-dung-ll-library-37LdeNXMVov
- Kiến thức về STM32F103: https://www.notion.so/STM32F103x-217555493c6080afb24bca6f639818be

## Thiết bị

- stm32
- ttp226
- nhip tim: Max30100
- MPU6050
- cảm biến nhiệt độ: DS18B20
- EEPROM: ic2431, DS2408 (ic mở rộng port)
- esp32-s3: wifi, ble
- màn hình st7789
- sd card (spi)
- Dùng freeRTOS để quản lí các task

gửi data dạng JSON, STM32F103C8T6 gửi dữ liệu qua ESP32 thông qua UART kết hợp DMA

Tìm hiểu cách viết thư viện JSON: https://github.com/DaveGamble/cJSON

https://github.com/microsoft/IoT-For-Beginners/tree/main
