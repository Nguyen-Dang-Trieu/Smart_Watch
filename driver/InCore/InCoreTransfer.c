#include "InCoreTransfer.h"

void IncoreTransfer_Init(void) {
#if defined(STM32)
    dma_usart_init_stm32();
#elif defined(ESP32)
    dma_uart_init_esp32();
#endif

}

// stm32 send
void IncoreTransfer_SendPacket(Packet_t* packetTX) {
    Packet_t currentPacket = *packetTX;
    dma_usart_send(currentPacket);
}

// esp32 receive
/*
 * Gọi 1 lần mỗi khi nhận được 1 byte
 * + Packet hoàn chỉnh => Trả về số byte nhận được.
 * + Packet chưa hoàn thành => Trả về 0.
 */
uint8_t IncoreTransfer_ReceivePacket(Packet_t* packetRX, uint8_t currentByte) {
    bool Packet_Complete = false;
    static uint8_t numberOfBytesReceived = 0;
	numberOfBytesReceived++;
	
    Packet_Complete = Packet_Parse(packetRX, currentByte);

    if (Packet_Complete) {
        uint8_t ret = numberOfBytesReceived;
        numberOfBytesReceived = 0;     // reset counter
        return ret;                    // trả về số byte trong packet
    }

    return 0; // Chưa hoàn thành nhận packet
}