#ifndef H_UART_DMA_ESP32_H
#define H_UART_DMA_ESP32_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define UART_PORT_NUM           UART_NUM_2          // Chọn UART2
#define UART_BAUD_RATE           115200              // Tốc độ baud  
#define UART_RX_RING_BUF_SIZE   (1024 * 2)           // 2KB - Size of RX ring buffer
#define QUEUE_SIZE              20


// Define UART pins
#define UART_TXD_PIN       (GPIO_NUM_17)         // Chân TX
#define UART_RXD_PIN       (GPIO_NUM_16)         // Chân RX
#define UART_RTS_PIN       (UART_PIN_NO_CHANGE)  // Không sử dụng RTS
#define UART_CTS_PIN       (UART_PIN_NO_CHANGE)  // Không sử dụng CTS


void uart_dma_esp32_init(void);

#endif // H_UART_DMA_ESP32_H