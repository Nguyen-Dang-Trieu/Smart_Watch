#ifndef PACKET_H
#define PACKET_H

/*
  Cấu trúc của Packet:
  [Start byte] [Packet ID] [payload length byte] [Payload bytes ... ] [8-bit CRC] [Stop byte].

  Trong đó:
  - Start byte: 1 byte có giá trị cố định là 0x7E
  - Packet ID: 1 byte định danh gói tin, mặc định là 0
  - payload length byte: 1 byte xác định số byte dữ liệu đã được mã hóa COBS trong packet
  - Payload bytes: Dữ liệu thực tế được truyền đi, tối đa 254 byte
  - 8-bit CRC: 1 byte kiểm tra lỗi dữ liệu
  - Stop byte: 1 byte có giá trị cố định là 0x81
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "crc8.h"
#include "cobs.h"

#define START_BYTE      0x7E
#define STOP_BYTE       0x81

#define PREAMBLE_SIZE   3	 	    // Start byte + Packet ID + payload length byte
#define POSTAMBLE_SIZE  2	      // 8-bit CRC + Stop byte
#define MAX_BUFFER_SIZE 0xFE    // Maximum allowed payload bytes per packet


/**
 * @brief Trạng thái xử lí packet
 */
typedef enum Packet_Processing_State {
    PACKET_WAIT_BYTE_START = 0,
    PACKET_WAIT_BYTE_ID,
    PACKET_WAIT_BYTE_LENGTH,
    PACKET_WAIT_PAYLOAD,
    PACKET_WAIT_BYTE_CRC,
    PACKET_WAIT_BYTE_STOP
} Packet_Processing_State_t;

/**
 * @brief Cấu trúc của Packet
 */
typedef struct Packet {
    uint8_t preamble[PREAMBLE_SIZE];
    uint8_t buffer[MAX_BUFFER_SIZE]; 
    uint8_t postamble[POSTAMBLE_SIZE]; 
} Packet_t;


/**
 * @brief Ghi dữ liệu vào trong buffer dưới dạng byte.
 * @param val Con trỏ đến biến cần ghi.
 * @param rawTxBuffer Mảng byte đích để ghi dữ liệu.
 * @param len Số byte cần ghi.
 * @return Số byte đã ghi vào mảng rawTxBuffer.
 */
uint8_t writeToBuffer(const void *val, uint8_t* rawTxBuffer, const uint8_t len);

/**
 * @brief Đọc dữ liệu từ buffer dưới dạng byte.
 * @param store Con trỏ đến biến đích để lưu dữ liệu.
 * @param rawRxBuffer Mảng byte nguồn để đọc dữ liệu.
 * @param len Số byte cần đọc.
 * @return Số byte đã đọc từ mảng rawRxBuffer.
 */
uint16_t readFromBuffer(void *store, uint8_t* rawRxBuffer, const uint8_t len);


void printBuffer(uint8_t *buffer, int len);

/**
 * @brief Tạo packet để truyền đi
 * @param packet Con trỏ đến cấu trúc Packet cần tạo.
 * @param messageLen Độ dài dữ liệu thực tế trong packet. (không bao gồm preamble và postamble), chưa mã hóa COBS
 * @param packetID ID của packet. (default là 0)
 * @return Trả về tổng độ dài của packet sau khi COBS.
 */
uint8_t Packet_Construct(Packet_t *txPacket, const uint8_t messageLen, const uint8_t packetID); 


/**
 * @brief Phân tích packet nhận được
 * @param packet Con trỏ đến cấu trúc Packet cần phân tích.
 * @param currentByteReceived Byte hiện tại nhận được.
 * @return true nếu packet hoàn chỉnh, false nếu chưa hoàn chỉnh.
 */
bool Packet_Parse(Packet_t *rxPacket, uint8_t currentByteReceived);



#endif // PACKET_H 