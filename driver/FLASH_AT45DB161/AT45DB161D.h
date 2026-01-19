#ifndef H_AT45DB161D_H
#define H_AT45DB161D_H

#include <stdbool.h>
#include <stddef.h>

#include <stdint.h>
#include "main.h"



/* Define pin - Chua chot cac chan, day chi la tam thoi de code thoi */
#define FLASH_CS_PORT      GPIOB
#define FLASH_CS_PIN       LL_GPIO_PIN_12

#define FLASH_WP_PORT      GPIOB
#define FLASH_WP_PIN       LL_GPIO_PIN_14

/* Control Pin */
#define FLASH_CS_LOW()     LL_GPIO_ResetOutputPin(FLASH_CS_PORT, FLASH_CS_PIN)
#define FLASH_CS_HIGH()    LL_GPIO_SetOutputPin(FLASH_CS_PORT, FLASH_CS_PIN)

// Public API
void AT45DB161_Init(void);

void AT45DB161_Mm_To_Buf(uint8_t BufferNum, uint16_t PageAddr);
void AT45DB161_Buf_To_Mm_WithErase(uint8_t BufferNum, uint16_t PageAddr);

void AT45DB161_ReadBuffer(uint8_t BufferNum, uint8_t *Buffer, uint16_t ReadAddr, uint16_t NumByteToRead);
void AT45DB161_WriteBuffer(uint8_t BufferNum, uint8_t *SrcBuffer, uint16_t BufferAddr, uint16_t NumByteToWrite);
void AT45DB161_PageRead(uint8_t *Buffer, uint16_t PageAddr, uint16_t NumByteToRead);

uint8_t  AT45DB161_GetStateReg(void);  
bool 	 AT45DB161_WaitReady(void); 
uint32_t AT45DB161_GetChipID(void);

void AT45DB161_ErasePage(uint16_t PageAddr);
void AT45DB161_EraseBlock(uint16_t BlockAddr);
void AT45DB161_EraseSector(uint8_t SectorAddr);
void AT45DB161_EraseChip(void);

#endif // H_AT45DB161D_H