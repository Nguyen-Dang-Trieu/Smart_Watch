#include "packet.h"

uint8_t writeToBuffer(const void *val, uint8_t* rawTxBuffer, const uint8_t len) {
	uint8_t* byteCurrent = (uint8_t*)val; // trỏ đến byte đầu tiên trong val
	uint8_t index = 0;
	uint8_t maxIndex;
	

  // Kiểm tra giới hạn vùng ghi 
	if ((len + index) > MAX_BUFFER_SIZE)
		maxIndex = MAX_BUFFER_SIZE;
	else
		maxIndex = len + index;

	for (uint8_t i = index; i < maxIndex; i++)
	{
	  /*
		txBuffer[i] = *byteCurrent;
		byteCurrent++;
		*/
		rawTxBuffer[i] = *byteCurrent++;

	}

	return maxIndex;
}

uint16_t readFromBuffer(void *store, uint8_t* rawRxBuffer, const uint8_t len) {
	uint8_t* byteCurrent = (uint8_t*)store;
	uint8_t maxIndex;
	uint8_t index = 0;

	if ((len + index) > MAX_BUFFER_SIZE)
		maxIndex = MAX_BUFFER_SIZE;
	else
		maxIndex = len + index;

	for (uint8_t i = index; i < maxIndex; i++) {
		*byteCurrent = rawRxBuffer[i];
		byteCurrent++;
		
		// *byteCurrent++ = rxBuffer[i];
	}

	return maxIndex;
}


uint8_t Packet_Construct(Packet_t *txPacket, const uint8_t messageLen, const uint8_t packetID)
{
	// Chưa xử lí và test trong dự án thực tế
    // Kiểm tra độ dài dữ liệu gửi có vượt quá kích thước tối đa không ?
	if (messageLen > MAX_BUFFER_SIZE)
	{
		// printf("Error: Message length exceeds MAX_BUFFER_SIZE\n");
        return -1;
	}
	else
	{
        // rawTxBuffer để chứa dữ liệu thô chưa mã hóa COBS
        uint8_t rawTxBuffer[messageLen];
        memcpy(rawTxBuffer, txPacket->buffer, messageLen);

		// txBufferCOBS để chứa dữ liệu đã mã hóa COBS
        uint8_t txBufferCOBS[COBS_MAX_LEN(messageLen)];

        // Mã hóa COBS
        int lenDataEncoded = COBS_Encode(rawTxBuffer, messageLen, txBufferCOBS);
		if(lenDataEncoded <= 0) {
			// printf("Error: COBS Encoding failed\n");
			return -1;
		}

		// Sao chép dữ liệu đã mã hóa COBS vào buffer của packet
		memcpy(txPacket->buffer, txBufferCOBS, lenDataEncoded);

        // Tính toán CRC8 kiểm tra lỗi toàn bộ payload
		uint8_t CRC8 = CRC8_TableLookup(txBufferCOBS, lenDataEncoded);

        // Xây dựng preamble và postamble
        txPacket->preamble[0] = START_BYTE;
		txPacket->preamble[1] = packetID;
		txPacket->preamble[2] = lenDataEncoded;

		txPacket->postamble[0] = CRC8;
        txPacket->postamble[1] = STOP_BYTE;

		return (uint8_t)lenDataEncoded;
	}
}


bool Packet_Parse(Packet_t *rxPacket, uint8_t currentByteReceived) {
	bool   Packet_IsComplete = false;
	static uint8_t bytesRead = 0;
	static Packet_Processing_State_t state = PACKET_WAIT_BYTE_START;

	switch(state) {
		case PACKET_WAIT_BYTE_START:
			// Lưu ID packet nếu cần
			rxPacket->preamble[0] = currentByteReceived; // Packet Start Byte
			state = PACKET_WAIT_BYTE_ID;
			break;

		case PACKET_WAIT_BYTE_ID:
			rxPacket->preamble[1] = currentByteReceived; // Packet ID
			state = PACKET_WAIT_BYTE_LENGTH;
			break;

		case PACKET_WAIT_BYTE_LENGTH:
			rxPacket->preamble[2] = currentByteReceived;
			state = PACKET_WAIT_PAYLOAD;
			break;

		case PACKET_WAIT_PAYLOAD:
			if(bytesRead < rxPacket->preamble[2]) {
				rxPacket->buffer[bytesRead++] = currentByteReceived;
				if(bytesRead >= rxPacket->preamble[2]) {
					state = PACKET_WAIT_BYTE_CRC;
				}	
			}
		
			break;

		case PACKET_WAIT_BYTE_CRC: {
			// Lưu byte CRC
			rxPacket->postamble[0] = currentByteReceived; // CRC byte

			// Kiểm tra CRC
			uint8_t buffer[rxPacket->preamble[2] + 1];
			memcpy(buffer, rxPacket->buffer, rxPacket->preamble[2]);
			buffer[rxPacket->preamble[2]] = rxPacket->postamble[0];

			uint8_t calculatedCRC = CRC8_TableLookup(buffer, sizeof(buffer));
			if(calculatedCRC == 0x00) {
				state = PACKET_WAIT_BYTE_STOP;
			} else {
				// CRC không hợp lệ, reset trạng thái
				bytesRead = 0;
				state = PACKET_WAIT_BYTE_START;
				printf("CRC Error!\n");
			}

			break;
		}
		
		case PACKET_WAIT_BYTE_STOP: 
		{
			if(currentByteReceived == STOP_BYTE) {
				// Packet hoàn chỉnh đã nhận
				// Xử lý packet ở đây nếu cần
				rxPacket->postamble[1] = currentByteReceived; // Stop byte
				state = PACKET_WAIT_BYTE_START;
				Packet_IsComplete = true;
				printf("Packet received successfully!\n");
			}
			break;
		}
		default:
			state = PACKET_WAIT_BYTE_START;
			break;
	}
	// Reset bytesRead = 0 if packet is complete, ready to receive new packet
	if(Packet_IsComplete) {
		bytesRead = 0; // Reset bytesRead khi hoàn thành packet
	}
	return Packet_IsComplete;
}


// Debug 
void printBuffer(uint8_t *buffer, int len) {
  for(int i = 0; i < len; i++) {
    printf("0x%02X ", buffer[i]);
  }
  printf("\n");
}
