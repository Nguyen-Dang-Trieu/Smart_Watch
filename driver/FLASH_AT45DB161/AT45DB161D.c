#include "AT45DB161D.h"
#include "AT45DB161D_cmds.h"
#include "stm32f1xx_delay.h"

void AT45DB161_Init(void) 
{
	FLASH_CS_HIGH(); // Luon o muc sao khi reset/power-up
	LL_delay_ms(10); // Cho flash on dinh

	// Can kiem tra page size cua flash la 512 hay 528 bytes / page truoc khi dung
	// Doc thanh ghi Status de kiem tra.
}

uint8_t AT45DB161_GetStateReg(void)
{
    uint8_t bytes_of_status = 0;
    FLASH_CS_LOW(); // Master keo CS xuong muc LOW
		
	// Wait until TX buffer empty
	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
	// Transmit data

	LL_SPI_TransmitData8(SPI1, CMD_STATUS_REG_READ);  //  Read Status Register: opcode 0xD7
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
    LL_SPI_ReceiveData8(SPI1); // discard dummy byte
	
	
	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
    LL_SPI_TransmitData8(SPI1, 0xFF); // send dummy byte to clock in status
	// Wait until RX buffer not empty
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {} ;
	// Nhan bytes du lieu
	bytes_of_status = LL_SPI_ReceiveData8(SPI1); 
			
	// Wait SPI idle
    while(LL_SPI_IsActiveFlag_BSY(SPI1)) {} // Kiem tra bit Busy flag, 1 = bus busy, 0 = bus idle

    FLASH_CS_HIGH(); // master keo CS len muc HIGH
    return bytes_of_status;
}

bool AT45DB161_WaitReady(void)
{
	uint16_t retry = 0;

  	while (retry < 100) {
		// Device ready to revice new comman
		if ((AT45DB161_GetStateReg() & STATUS_READY_BIT) == true) {
            return true;
		}

    	retry++;
		LL_delay_us(50); // Khong than thien voi freeRTOS se dieu chinh lai sau
  }
  return false;
}

uint32_t AT45DB161_GetChipID(void)
{
	uint32_t result = 0;
	
	FLASH_CS_LOW(); // Master keo CS xuong muc LOW
	
	/* 1. Gui command 0x9F */
	// Wait until TX buffer empty
	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
	// Transmit data
	LL_SPI_TransmitData8(SPI1, CMD_MANU_AND_DEVICE_ID_READ);  
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
    LL_SPI_ReceiveData8(SPI1); // discard dummy byte
	
	/* 2. Nhan data co format 4byte: 1FH -> device id bytes 1 -> device id bytes 2 -> 00H  */
	uint8_t cur_bytes_in_buf = 0; // Bytes data hien tai trong buffer RX SPI
	for(uint8_t i = 0; i < 4; i++) {
		while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
		LL_SPI_TransmitData8(SPI1, 0xFF); // send dummy byte to clock in status
					
		// Wait until RX buffer not empty
		while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {} ;
		// Nhan bytes du lieu
		cur_bytes_in_buf = LL_SPI_ReceiveData8(SPI1); 
					
		result = (result << 8) | cur_bytes_in_buf;
					
	}
		
	// Wait SPI idle
    while(LL_SPI_IsActiveFlag_BSY(SPI1)) {}
			
	FLASH_CS_HIGH(); // master keo CS len muc HIGH
    return result;
}

/**
@brief Main Memory Page to Buffer Transfer
*/
void AT45DB161_Mm_To_Buf(uint8_t BufferNum, uint16_t PageAddr)
{
	if (!AT45DB161_WaitReady()) return; // wait for device to ready
	if (PageAddr >= FLASH_TOTAL_PAGES) return;
	
	uint8_t cmd = 0;
		
	FLASH_CS_LOW(); // master keo CS xuong LOW
		
	/* 1. Send command */
	if(BufferNum == FLASH_USE_INTERNAL_BUF1) {
		cmd = CMD_MAIN_MEM_TO_BUF1_TRANS;
					
	} else { // BufferNum == FLASH_USE_INTERNAL_BUF2
		cmd = CMD_MAIN_MEM_TO_BUF2_TRANS;
	}
	
	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
	LL_SPI_TransmitData8(SPI1, cmd);  
						
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}; 
	LL_SPI_ReceiveData8(SPI1); // discard dummy byte - can thiet de don rac buffer
	
	/*2. Gui 3 byte Page Address */
	uint8_t addrBytes[3];
	addrBytes[0] = (uint8_t )(PageAddr >> 6);
	addrBytes[1] = (uint8_t )((PageAddr & 0x3F) << 2);
	addrBytes[2] = 0x00;

	for(uint8_t i = 0; i < 3; i++) {
		while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
    	LL_SPI_TransmitData8(SPI1, addrBytes[i]);
			
    	while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
		LL_SPI_ReceiveData8(SPI1); // discard dummy byte
	}
		
	// Wait SPI idle
  	while(LL_SPI_IsActiveFlag_BSY(SPI1)) {}
			
	FLASH_CS_HIGH(); // master keo CS len muc HIGH
}

/**
 @brief Buffer to Main Memory Page Program with Built-in Erase
*/
void AT45DB161_Buf_To_Mm_WithErase(uint8_t BufferNum, uint16_t PageAddr)
{
	if (PageAddr >= FLASH_TOTAL_PAGES) return;
	while(!AT45DB161_WaitReady()); // wait for device to ready
	
	uint8_t cmd = 0; // Command
		
	FLASH_CS_LOW(); // master keo CS xuong LOW
		
	/* 1. Send command */
	if(BufferNum == FLASH_USE_INTERNAL_BUF1) {
		cmd = CMD_BUF1_TO_MAIN_MEM_WITH_ERASE;
					
	} else { // BufferNum == FLASH_USE_INTERNAL_BUF2
		cmd = CMD_BUF2_TO_MAIN_MEM_WITH_ERASE;
	}
	
	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
	LL_SPI_TransmitData8(SPI1, cmd);  
						
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}; 
	LL_SPI_ReceiveData8(SPI1); // discard dummy byte - can thiet de don rac buffer
	
	/*2. Gui 3 byte Page Address */
	uint8_t addrBytes[3];
	addrBytes[0] = (uint8_t )(PageAddr >> 6);			 	// 6 bits cao cua Page Address
	addrBytes[1] = (uint8_t )((PageAddr & 0x003F) << 2);  	// 6 bits thap cua Page Address
	addrBytes[2] = 0x00;

	for(uint8_t i = 0; i < 3; i++) {
		while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
    	LL_SPI_TransmitData8(SPI1, addrBytes[i]);
			
    	while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
		LL_SPI_ReceiveData8(SPI1); // discard dummy byte
	}
		
	// Wait SPI idle
  	while(LL_SPI_IsActiveFlag_BSY(SPI1)) {}
			
	FLASH_CS_HIGH(); // master keo CS len muc HIGH
}

/**
@brief Read data in specified AT45DB161 internal buffer
@param BufferNum Lua chon buffer 1 or buffer 2 de doc
@param Buffer 	 Bo dem de chua du lieu
@param BufferAddr  Dia chi bat dau doc trong buffer
@param NumByteToRead So lan bytes can doc

*/
void AT45DB161_ReadBuffer(uint8_t BufferNum, uint8_t *Buffer, uint16_t BufferAddr, uint16_t NumByteToRead)
{
	// Kiem tra cac tham so dau vao
	if (BufferAddr >= FLASH_PAGE_SIZE || Buffer == NULL) return; // ReadAddr >= 528
	if(BufferAddr + NumByteToRead >= FLASH_PAGE_SIZE)
		 NumByteToRead = FLASH_PAGE_SIZE - BufferAddr;
	
	while(!AT45DB161_WaitReady()); // wait for device to ready
	
	uint8_t cmd = 0; // Command
	FLASH_CS_LOW();  // master keo CS xuong LOW
		
	/* 1. Send command */
	if(BufferNum == FLASH_USE_INTERNAL_BUF1) {
		cmd = CMD_BUF_1_READ;
					
	} else { // BufferNum == FLASH_USE_INTERNAL_BUF2
		cmd = CMD_BUF_2_READ;
	}
	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
	LL_SPI_TransmitData8(SPI1, cmd);  
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}; 
	LL_SPI_ReceiveData8(SPI1); // discard dummy byte - can thiet de don rac buffer
	
		
	// Opcode: D4H va D6H, them 1 byte dummy -> Total 4 bytes	dia chi
	// Opocde: D1H va D3H, khg can 1 byte dummy -> Total 3 bytes dia chi	
	/* 2. Send 4 bytes address de xac dinh vi tri doc trong buffer */
	uint8_t bytesAddr[4];
	bytesAddr[0] = 0x00;
	bytesAddr[1] = (uint8_t )((BufferAddr & 0x0300) >> 8); 	// 2 bits cao cua Byte Address
	bytesAddr[2] = (uint8_t )(BufferAddr & 0x00FF);			// 8 bits thap cua Bytes Address
	bytesAddr[3] = 0xFF; // Bytes dummy
		
	for(uint8_t i = 0; i < 4; i++) {
		while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
    	LL_SPI_TransmitData8(SPI1, bytesAddr[i]);
			
    	while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
		LL_SPI_ReceiveData8(SPI1); // discard dummy byte
	}	
		
	/* 3. Doc byte tu buffer vao RAM */
	for(uint16_t b = 0; b < NumByteToRead; b++){
		while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
    	LL_SPI_TransmitData8(SPI1, 0xFF); // send dummy byte to clock in status
		
		while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {} ; // Wait until RX buffer not empty
		Buffer[b] = LL_SPI_ReceiveData8(SPI1); // Nhan bytes du lieu
	}
	
	/* 4. End */
 	while(LL_SPI_IsActiveFlag_BSY(SPI1)) {} // Wait SPI idle
	FLASH_CS_HIGH(); // master keo CS len muc HIGH
}

/**
@brief Write data to specified AT45DB161 internal buffer
@param BufferNum Lua chon buffer 1 hay buffer 2
@param SrcBuffer Noi du lieu dang duoc luu tru
@param BufferAddr Vi tri byte bat dau ghi trong buffer
@param NumByteToWrite So luong bytes can ghi

@note

- Nen su dung ham nay khi byte du lieu con chinh sua nhieu. Co the dung buffer lam noi ghi tam thoi khi da 
ok thi moi ghi vao trong main memory.

*/
void AT45DB161_WriteBuffer(uint8_t BufferNum, uint8_t *SrcBuffer, uint16_t BufferAddr, uint16_t NumByteToWrite)
{
	if(BufferAddr >= FLASH_PAGE_SIZE || SrcBuffer == NULL) return;
	if(BufferAddr + NumByteToWrite > FLASH_PAGE_SIZE) NumByteToWrite = FLASH_PAGE_SIZE - BufferAddr;
	
  	// Start
	while(!AT45DB161_WaitReady()) {}; // wait for device to ready
	FLASH_CS_LOW();
		
	/* 1. Send command */
	uint8_t cmd = 0;
		
 	if (BufferNum == FLASH_USE_INTERNAL_BUF1) {
		cmd = CMD_BUF_1_WRITE;
 	} else {
      	cmd = CMD_BUF_2_WRITE;
  	}
	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
	LL_SPI_TransmitData8(SPI1, cmd);  
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}; 
	LL_SPI_ReceiveData8(SPI1); // discard dummy byte - can thiet de don rac buffer
	
	/* 2. Send Byte address */
	uint8_t bytesAddr[3];
	bytesAddr[0] = 0x00;
	bytesAddr[1] = (uint8_t )((BufferAddr & 0x0300) >> 8); 	 // 2 bits cao cua Byte Address
	bytesAddr[2] = (uint8_t )( BufferAddr & 0x00FF);	 	 // 8 bits thap cua Byte Address

	for(uint8_t i = 0;i < 3; i++) {
		while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
    	LL_SPI_TransmitData8(SPI1, bytesAddr[i]);
			
    	while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
		LL_SPI_ReceiveData8(SPI1); // discard dummy byte
	}
	
	/* 3. Write data */
	for(uint16_t b = 0; b < NumByteToWrite; b++) {
		while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
    	LL_SPI_TransmitData8(SPI1, SrcBuffer[b]);
			
    	while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
		LL_SPI_ReceiveData8(SPI1); // discard dummy byte
	}

	/* 4. End */
  	while(LL_SPI_IsActiveFlag_BSY(SPI1)) {} // Wait SPI idle
	FLASH_CS_HIGH(); // master keo CS len muc HIGH

}


/**
@brief Read data from specified page address
@param Buffer Noi luu tru data
@param PageAddr Dia chi cua page (0 - > 4095)
@param NumByteToRead So luong byte can doc

@note
- Doc toan bo data trong page 
- Ham nay doc data truc tiep tu main memory
- Khong thong qua buffer 1 hay buffer 2 
*/
void AT45DB161_PageRead(uint8_t *Buffer, uint16_t PageAddr, uint16_t NumByteToRead)
{
    if(PageAddr >= FLASH_TOTAL_PAGES || Buffer == NULL) return;
	
    if(NumByteToRead > FLASH_PAGE_SIZE) NumByteToRead = FLASH_PAGE_SIZE;
    while(!AT45DB161_WaitReady()); // wait for device to ready
	
	// Start
	FLASH_CS_LOW();
	
	/*1. Send command CMD_MAIN_MEM_PAGE_READ = 0xD2 */
    while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
	LL_SPI_TransmitData8(SPI1, CMD_MAIN_MEM_PAGE_READ);  
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}; 
	LL_SPI_ReceiveData8(SPI1); 
	
	/*2. Send Addr */
	uint8_t bytesAddr[7]; // 3 bytes addr + 4 bytes dummy
	bytesAddr[0] = (uint8_t )(PageAddr >> 6);						// 6 bytes cua page address
		
	// Important node: Hien tai doc full page -> byte address khong su dung -> 10 bytes cua bytes addrss = 0
	bytesAddr[1] = (uint8_t)((PageAddr & 0x003F) << 2);	// 6 bytes thap cua page address + 2 bytes cao cua byte address
	bytesAddr[2] = 0x00;				 												// 8 bits thap cua Byte Address
	bytesAddr[3] = 0xFF; // Byte dummy
	bytesAddr[4] = 0xFF; // Byte dummy
	bytesAddr[5] = 0xFF; // Byte dummy
	bytesAddr[6] = 0xFF; // Byte dummy
			
	for(uint8_t i = 0; i < 7; i++) {
		while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
		LL_SPI_TransmitData8(SPI1, bytesAddr[i]);
			
		while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
		LL_SPI_ReceiveData8(SPI1); // discard dummy byte
	}
		
	/* 3. Doc byte du lieu tu main memory vao RAM */
	for(uint16_t b = 0; b < NumByteToRead; b++){
		while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
		LL_SPI_TransmitData8(SPI1, 0xFF); // send dummy byte to clock in status
		
		while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {} ; // Wait until RX buffer not empty
		Buffer[b] = LL_SPI_ReceiveData8(SPI1); // Nhan bytes du lieu
	}
    
    /* 4. End */
	while(LL_SPI_IsActiveFlag_BSY(SPI1)) {} // Wait SPI idle
	FLASH_CS_HIGH(); // master keo CS len muc HIGH
}

/**
@brief Erase specified page
@param PageAddr Dia chi cua page muon xoa (PageAddr < 4096)
*/
void AT45DB161_ErasePage(uint16_t PageAddr)
{
	// Kiem tra cac tham so dau vao
	if(PageAddr >= FLASH_TOTAL_PAGES) return;
  	while(!AT45DB161_WaitReady()) {};
    
	// Start
	FLASH_CS_LOW();
  
	/*1. Send command CMD_PAGE_ERASE = 0x81 */
  	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
	LL_SPI_TransmitData8(SPI1, CMD_PAGE_ERASE);  
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}; 
	LL_SPI_ReceiveData8(SPI1); 
		
	/* 2. Send Page address: 12 bits */
	uint8_t bytesAddr[3];
	bytesAddr[0] = (uint8_t )(PageAddr >> 6); // xx-PA11 -> PA6: 6 bits cao cua Page Addr
	bytesAddr[1] = (uint8_t )((PageAddr & 0x003F) << 2); // PA5->PA0-xx: 6 bits thap cua Page Addr
	bytesAddr[2] = 0x00;		
		
	for(uint8_t i = 0;i < 3; i++) {
		while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
    	LL_SPI_TransmitData8(SPI1, bytesAddr[i]);
			
    	while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
		LL_SPI_ReceiveData8(SPI1); // discard dummy byte
	}
	
  	/* 4. End */
  	while(LL_SPI_IsActiveFlag_BSY(SPI1)) {} // Wait SPI idle
	FLASH_CS_HIGH(); // master keo CS len muc HIGH
		
	// Important Note: Khi CS chuyen LOW -> HIGH, qua trinh erase moi thuc su bat dau (canh len cua CS).
}

/**
@brief Erase specified block
@param BlockAddr Dia chi cua block can xoa (BlockAddr < 512)

*/
void AT45DB161_EraseBlock(uint16_t BlockAddr)
{
	if(BlockAddr >= FLASH_TOTAL_BLOCKS) return;
  	while(!AT45DB161_WaitReady()) {};
    
  	// Start
	FLASH_CS_LOW();
		
	/*1. Send command CMD_BLOCK_ERASE = 0x50 */
 	while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
	LL_SPI_TransmitData8(SPI1, CMD_BLOCK_ERASE);  
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}; 
	LL_SPI_ReceiveData8(SPI1); 
		
	/* 2. Send Block address: 9 bits */
	uint8_t bytesAddr[3];
	bytesAddr[0] = (uint8_t )(BlockAddr >> 3); 					 		// xx-PA11 -> PA6: 6 bits cao cua Block Addr
	bytesAddr[1] = (uint8_t )((BlockAddr & 0x0007) << 5); // PA5->PA3-xxxxx: 3 bits thap cua Block Addr
	bytesAddr[2] = 0x00;		
		
	for(uint8_t i = 0;i < 3; i++) {
		while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
    	LL_SPI_TransmitData8(SPI1, bytesAddr[i]);
			
    	while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
		LL_SPI_ReceiveData8(SPI1); // discard dummy byte
	}
	
  	/* 3. End */
  	while(LL_SPI_IsActiveFlag_BSY(SPI1)) {} // Wait SPI idle
	FLASH_CS_HIGH(); // master keo CS len muc HIGH
}

/**
@brief Erase specified sector
@param SectorAddr Dia chi cua sector can xoa (SectorAddr < 16)
*/
void AT45DB161_EraseSector(uint8_t SectorAddr)
{
	// Kiem tra tham so dau vao
	if(SectorAddr >= FLASH_TOTAL_SECTORS) return;
	
	while(!AT45DB161_WaitReady()) {};

	// Note: Sector 0 co 2 vung khac nhau
  	if(SectorAddr == 0) {
    	// Erase 0a sector of sector 0
    	FLASH_CS_LOW();
						
		/*1. Send command CMD_SECTOR_ERASE = 0x7C */
		while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
		LL_SPI_TransmitData8(SPI1, CMD_SECTOR_ERASE);  
		while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}; 
		LL_SPI_ReceiveData8(SPI1); 
		
		/* 2. Send Block address: 9 bits */
		uint8_t bytesAddr[3];
		bytesAddr[0] = 0x00; 					 		
		bytesAddr[1] = 0x00; 
		bytesAddr[2] = 0x00;		
		
		for(uint8_t i = 0;i < 3; i++) {
			while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
			LL_SPI_TransmitData8(SPI1, bytesAddr[i]);
			
			while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
			LL_SPI_ReceiveData8(SPI1); // discard dummy byte
		}
						
    	FLASH_CS_HIGH(); 
    	LL_delay_ms(1); // 1ms
			
    	// Erase 0b sector of sector 0
		while(!AT45DB161_WaitReady()) {}; // Cho xoa xong a0 cua sector 0
    	FLASH_CS_LOW();
		
		/*1. Send command CMD_SECTOR_ERASE = 0x7C */
		while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
		LL_SPI_TransmitData8(SPI1, CMD_SECTOR_ERASE);  
		while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}; 
		LL_SPI_ReceiveData8(SPI1);	
        
		bytesAddr[0] = 0x00; 					 		
		bytesAddr[1] = 0x20; 
		bytesAddr[2] = 0x00;		
		
		for(uint8_t i = 0;i < 3; i++) {
			while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
			LL_SPI_TransmitData8(SPI1, bytesAddr[i]);
			
			while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
			LL_SPI_ReceiveData8(SPI1); // discard dummy byte
		}
		
		// End
		while(LL_SPI_IsActiveFlag_BSY(SPI1)) {} // Wait SPI idle
		FLASH_CS_HIGH(); 
			
	} else {
      	// Start
		FLASH_CS_LOW();
		
		/*1. Send command CMD_SECTOR_ERASE = 0x7C */
		while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
		LL_SPI_TransmitData8(SPI1, CMD_SECTOR_ERASE);  
		while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}; 
		LL_SPI_ReceiveData8(SPI1);	
        
		/* 2. Send Block address: 9 bits */
		uint8_t bytesAddr[3];
		bytesAddr[0] = (uint8_t )((SectorAddr & 0x0F) << 2); 					 		
		bytesAddr[1] = 0x00; 
		bytesAddr[2] = 0x00;	
			
		for(uint8_t i = 0;i < 3; i++) {
			while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
			LL_SPI_TransmitData8(SPI1, bytesAddr[i]);
			
			while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
			LL_SPI_ReceiveData8(SPI1); // discard dummy byte
		}
			
		/* 3. End */
		while(LL_SPI_IsActiveFlag_BSY(SPI1)) {} // Wait SPI idle
		FLASH_CS_HIGH(); // master keo CS len muc HIGH
  }
}

/**
@brief Chip Erase
*/
void AT45DB161_EraseChip(void)
{
	while(!AT45DB161_WaitReady()) {};
  	// Start
	FLASH_CS_LOW();

	/* Send bytes opcode: 4 bytes*/
	uint8_t bytesOpcode[4];
 	bytesOpcode[0] = CMD_CHIP_ERASE[0]; 					 		
	bytesOpcode[1] = CMD_CHIP_ERASE[1]; 
	bytesOpcode[2] = CMD_CHIP_ERASE[2];	
	bytesOpcode[3] = CMD_CHIP_ERASE[3];			
		
	for(uint8_t i = 0; i < 4; i++) {
		while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {};
		LL_SPI_TransmitData8(SPI1, bytesOpcode[i]);
			
		while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {};
		LL_SPI_ReceiveData8(SPI1); // discard dummy byte
	}
		
	/*  End */
	while(LL_SPI_IsActiveFlag_BSY(SPI1)) {} // Wait SPI idle
	FLASH_CS_HIGH(); // master keo CS len muc HIGH
}
