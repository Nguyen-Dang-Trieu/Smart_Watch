#ifndef H_AT45DB161D_CMDS_H
#define H_AT45DB161D_CMDS_H

// Vi hardware cua AT45DB161D khong ho tro CS (Chip select) nen ta dung GPIO de kich hoat
// datasheet: https://www.micros.com.pl/mediaserver/info-pee45db161d-su.pdf
// source tham khao: https://github.com/coocox/cox/tree/master/CoX/Driver/Memory_Flash_Atmel/AT45DB161/AT45DB161_Single/lib

#include <stdint.h>

/* Command Tables */
// Read commands
#define CMD_MAIN_MEM_PAGE_READ	    0xD2 // Main Memory Page Read
#define CMD_CONTI_ARR_READ			0xE8 // Continuous Array Read (Legacy Command) - Khong khuyen khich su dung
#define CMD_CONTI_ARR_READ_L		0x03 // Continuous Array Read (Low Frequency)
#define CMD_CONTI_ARR_READ_H 		0x0B // Continuous Array Read (High Frequency)
#define CMD_BUF_1_READ_LOW 			0xD1 // Buffer 1 Read (Low Frequency)
#define CMD_BUF_2_READ_LOW 			0xD3 // Buffer 2 Read (Low Frequency)
#define CMD_BUF_1_READ		 		0xD4 // Buffer 1 Read
#define CMD_BUF_2_READ		 		0xD6 // Buffer 2 Read

// Program and Erase Commands
#define CMD_BUF_1_WRITE 					    0x84 // Buffer 1 Write
#define CMD_BUF_2_WRITE 						0x87 // Buffer 2 Write
#define CMD_BUF1_TO_MAIN_MEM_WITH_ERASE 		0x83 // Buffer 1 to Main Memory Page Program with Built-in Erase
#define CMD_BUF2_TO_MAIN_MEM_WITH_ERASE 		0x86 // Buffer 2 to Main Memory Page Program with Built-in Erase
#define CMD_BUF1_TO_MAIN_MEM_WITHOUT_ERASE 	    0x88 // Buffer 1 to Main Memory Page Program without Built-in Erase
#define CMD_BUF2_TO_MAIN_MEM_WITHOUT_ERASE 	    0x89 // Buffer 2 to Main Memory Page Program without Built-in Erase
#define CMD_PAGE_ERASE 							0x81
#define CMD_BLOCK_ERASE 						0x50
#define CMD_SECTOR_ERASE 						0x7C
#define CMD_MAIN_MEM_THROUGH_BUF1 				0x82 // Main Memory Page Program Through Buffer 1
#define CMD_MAIN_MEM_THROUGH_BUF2 				0X85 // Main Memory Page Program Through Buffer 2

static const uint8_t CMD_CHIP_ERASE[4] = {0xC7, 0x94, 0x80, 0x9A};

// Protection and Security Commands
#define CMD_READ_SEC_PRO_REG 			0x32 // Read Sector Protection Register
#define CMD_READ_SEC_LOKDOW_REG 	    0x35 // Read Sector Lockdown Register
#define CMD_READ_SECURITY 				0x77 // Read Security Register

static const uint8_t CMD_ENA_SEC_PROTECT[4]   = {0x3D, 0x2A, 0x7F, 0xA9}; // Enable Sector Protection
static const uint8_t CMD_DIS_SEC_PROTECT[4]   = {0x3D, 0x2A, 0x7F, 0x9A}; // Disable Sector Protection
static const uint8_t CMD_ERASE_SEC_PRO_REG[4] = {0x3D, 0x2A, 0x7F, 0xCF}; // Erase Sector Protection Register
static const uint8_t CMD_PROG_SEC_PRO_REG[4]  = {0x3D, 0x2A, 0x7F, 0xFC}; // Program Sector Protection Register
static const uint8_t CMD_PROGRAM_SECURITY[4]  = {0x9B, 0x00, 0x00, 0x00}; // Program Security Register
static const uint8_t CMD_SEC_LOCKDOWN[4]  	  = {0x3D, 0x2A, 0x7F, 0x30}; // Sector Lockdown

// ---- Additional Commands ----
#define CMD_MAIN_MEM_TO_BUF1_TRANS 			0x53 // Main Memory Page to Buffer 1 Transfer
#define CMD_MAIN_MEM_TO_BUF2_TRANS 			0x55 // Main Memory Page to Buffer 2 Transfer
#define CMD_MAIN_MEM_TO_BUF1_COMPARE 		0x60 // Main Memory Page to Buffer 1 Compare
#define CMD_MAIN_MEM_TO_BUF2_COMPARE 		0x61 // Main Memory Page to Buffer 2 Compare
#define CMD_AUTO_PAGE_REW_THROUGH_BUF1      0x58 // Auto Page Rewrite through Buffer 1
#define CMD_AUTO_PAGE_REW_THROUGH_BUF2      0x59 // Auto Page Rewrite through Buffer 2
#define CMD_DEEP_POWER_DOWN 				0xB9 // Deep Power-down
#define CMD_RES_FROM_DPD   					0xAB // Resume from Deep Power-down
#define CMD_STATUS_REG_READ 				0xD7 // Status Register Read
#define CMD_MANU_AND_DEVICE_ID_READ 		0x9F // Manufacturer and Device ID Read

/* BitMask cua Status Reg Read */
#define STATUS_READY_BIT          0x80 // Bit 7: 1 = Ready, 0 = Busy
#define STATUS_COMP_BIT           0x40 // Bit 6: 0 = Match, 1 = Not Match
#define STATUS_PROTECT_BIT        0x02 // Bit 1: 1 = Enabled, 0 = Disabled
#define STATUS_PAGE_SIZE_BIT      0x01 // Bit 0: 1 = 512 bytes (Binary), 0 = 528 bytes (Standard)

// Flash specifications
#define FLASH_MAX_CLK 					66000000 	// 66 MHz
#define FLASH_PAGE_SIZE_DEFAULT 		528 		// Bytes
#define FLASH_PAGE_SIZE_BINARY			512 		// Bytes
#define FLASH_PAGE_SIZE    				FLASH_PAGE_SIZE_DEFAULT // Hien tai use page size = 528, co the modify

#define FLASH_TOTAL_PAGES				4096

#define PAGES_PER_BLOCK					8
#define FLASH_TOTAL_BLOCKS				(FLASH_TOTAL_PAGES / PAGES_PER_BLOCK) // Total = 512 blocks

#define FLASH_TOTAL_SECTORS				16

#define FLASH_USE_INTERNAL_BUF1			1	
#define FLASH_USE_INTERNAL_BUF2			2


#endif // H_AT45DB161D_CMDS_H