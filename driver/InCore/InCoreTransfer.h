#ifndef INCORE_TRANSFER_H
#define INCORE_TRANSFER_H

#include "packet.h"

#if defined(STM32)
    #include "dma_usart_stm32.h"
#elif defined(ESP32)
    #include "dma_usart_esp32.h"
#endif

/**
 * @brief Khởi tạo Incore Transfer Protocol
 * @note Gọi hàm init của platform phần cứng phù hợp
 */
void IncoreTransfer_Init(void);

/**
 * @brief Gửi Packet qua giao tiếp Incore Transfer Protocol
 * 
 * @param packet Con trỏ đến Packet cần gửi.
 * @return none
 */
void IncoreTransfer_SendPacket(Packet_t* packetTX);

/**
 * @breif Nhận Packet từ giao tiếp Incore Transfer Protocol
 * @param packetRX Con trỏ đến Packet nhận được
 * @return uint8_t Số byte đã đọc được
 */
uint8_t IncoreTransfer_ReceivePacket(Packet_t* packetRX);

#endif // INCORE_TRANSFER_H