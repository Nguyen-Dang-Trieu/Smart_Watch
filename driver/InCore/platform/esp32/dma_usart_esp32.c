#include "uart_dma_esp32.h"

static QueueHandle_t uart2_queue;
static const char *TAG = "UART_DMA";

void uart_dma_esp32_init(void) {
    const uart_config_t uart_config = {
        .baud_rate  = UART_BAUD_RATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE, 
        .source_clk = UART_SCLK_DEFAULT,
    };

    /* 1. Cài đặt các tham số uart */
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));

    /* 2. Định nghĩa các chân GPIO */
    ESP_LOGI(TAG, "Setting UART%d pins: TXD=%d, RXD=%d", UART_PORT_NUM, UART_TXD_PIN, UART_RXD_PIN);
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TXD_PIN, UART_RXD_PIN, UART_RTS_PIN, UART_CTS_PIN));

    /* 3. Cài đặt driver UART, cấp phát bộ đệm và tạo hàng đợi sự kiện */
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_RX_RING_BUF_SIZE, 0, QUEUE_SIZE, &uart2_queue, 0));

    /* 4. Kích hoạt tính năng ngắt/sự kiện nhận đủ số byte (Pattern Detect) */
	// Cần phải thay đổi thông số thứ 2 của hàm uart_set_rx_full_thresould(,...)
    uart_set_rx_full_threshold(UART_PORT_NUM, 20); // Gửi sự kiện khi có 20 byte trong đệm
}

