#include <stdio.h>
#include <stdint.h>

#define MAX_PACKET_SIZE 0xFE // 254

uint8_t txBuffer[MAX_PACKET_SIZE];
uint8_t rxBuffer[MAX_PACKET_SIZE];

uint8_t writeToTxBuffer(const int *val, const uint8_t len) {
	uint8_t* byteCurrent = (uint8_t*)val; // trỏ đến byte đầu tiên trong val
	uint8_t index = 0;
	uint8_t maxIndex;
	

  // Kiểm tra giới hạn vùng ghi 
	if ((len + index) > MAX_PACKET_SIZE)
		maxIndex = MAX_PACKET_SIZE;
	else
		maxIndex = len + index;

	for (uint8_t i = index; i < maxIndex; i++)
	{
	  /*
		txBuffer[i] = *byteCurrent;
		byteCurrent++;
		*/
		txBuffer[i] = *byteCurrent++;

	}

	return maxIndex;
}


uint16_t readFromRxBuffer(int *store, const uint8_t len) {
	uint8_t* byteCurrent = (uint8_t*)store;
	uint8_t maxIndex;
	uint8_t index = 0;

	if ((len + index) > MAX_PACKET_SIZE)
		maxIndex = MAX_PACKET_SIZE;
	else
		maxIndex = len + index;

	for (uint8_t i = index; i < maxIndex; i++) {
		*byteCurrent = rxBuffer[i];
		byteCurrent++;
		
		// *byteCurrent++ = rxBuffer[i];
	}

	return maxIndex;
}



// Debug 
void printBuffer(uint8_t *txBuffer, int len) {
  for(int i = 0; i < len; i++) {
    printf("0x%02X ", txBuffer[i]);
  }
  printf("\n");
}


int main()
{
    // printf("Kich thuoc cua int: %ld byte\n", sizeof(int));

    int a = 0x12345678; // giá trị mẫu
    printf("Ghi gia tri a = 0x%X vao txBuffer...\n", a);

    uint8_t bytesWritten = writeToTxBuffer(&a, sizeof(a));
    printf("So byte da ghi: %d\n", bytesWritten);

    printf("txBuffer: ");
    printBuffer(txBuffer, bytesWritten);

    // Giả lập truyền nhận (copy tx → rx): UART
    for (int i = 0; i < bytesWritten; i++)
        rxBuffer[i] = txBuffer[i];

    // Đọc lại dữ liệu
    int b = 0;
    readFromRxBuffer(&b, sizeof(b));
    printf("Giá trị đọc lại: 0x%X\n", b);

    return 0;

} 
