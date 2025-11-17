#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "crc8.h"


int main() {
    // Dữ liệu mẫu để tính toán CRC8
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    int dataSize = sizeof(data) / sizeof(data[0]);

    //  Tính toán CRC8
    // C1: Sử dụng bitwise method
    uint8_t crcValueBitWise = CRC8_BitWise(data, dataSize, 0x97);

    // C2: Sử dụng table lookup method
    uint8_t crcValueTableLookup = CRC8_TableLookup(data, dataSize);

    // Copy byte CRC8 vào trong buffer
    uint8_t buffer[dataSize + 1];
    memcpy(buffer, data, dataSize);
    buffer[dataSize] = crcValueBitWise;

    // In kết quả
    printf("CRC8 Value BitWise: 0x%02X\n", crcValueBitWise);


    // Kiểm tra tính đúng đắn
    uint8_t check_CRC_bitwise = CRC8_BitWise(buffer, dataSize + 1, 0x97);
    uint8_t check_CRC_tablelookup = CRC8_TableLookup(buffer, dataSize + 1);

    printf("CRC Check (should be 0) - bitwise: 0x%02X\n", check_CRC_bitwise);
    printf("CRC Check (should be 0) - table: 0x%02X\n", check_CRC_tablelookup);

    return 0;
}