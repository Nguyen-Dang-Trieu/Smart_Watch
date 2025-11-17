#ifndef CRC8_H
#define CRC8_H

#include <stdint.h>
#include <stdio.h>

// Poject tham khảo: https://github.com/hdtodd/CRC8-Library/blob/main/libcrc8.c

/* Da thuc G(x) = x^7 + x^4 + x^2 + x + 1 */
extern uint8_t CRC8_Polynomial; 
extern uint8_t CRC8_Table[256];

uint8_t CRC8_BitWise(uint8_t *data, int size, uint8_t poly);
void CRC8_BuildTable(uint8_t poly, uint8_t* CRC8_Table);
uint8_t CRC8_TableLookup(uint8_t *data, int size_data);

#endif // CRC8_H