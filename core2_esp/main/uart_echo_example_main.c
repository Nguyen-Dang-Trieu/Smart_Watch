#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "uart_dma_esp32.h"

QueueHandle_t PacketRx_queue;


static void uart_task(void *arg)
{
    Packet_t packet_rx;
    uart_event_t event;
    uint8_t byte;

    while (1) {
        if (xQueueReceive(uart2_queue, &event, portMAX_DELAY)) {

            if (event.type == UART_DATA) {

                for (int i = 0; i < event.size; i++) {

                    uart_read_bytes(UART_PORT_NUM, &byte, 1, portMAX_DELAY);

                    uint8_t result = IncoreTransfer_ReceivePacket(&packet_rx, byte);

                    if (result > 0) {
                        // packet complete
                        xQueueSend(PacketRx_queue, &packet_rx, 0);
                    }
                }
            }
        }
    }
}


void app_main(void) {
    uart_dma_esp32_init();

    xTaskCreate(uart_task, "uart_task", 4096, NULL, 10, NULL);

    Packet_t packet;

    while (1) {
        if (xQueueReceive(PacketRx_queue, &packet, portMAX_DELAY)) {
            printf("Packet received!\n");
        }
    }
}
