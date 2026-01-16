#include "flash.h"

uint8_t FLASH[AT45_TOTAL_PAGES * AT45_PAGE_SIZE] = {0};

void AT45DB_Init(void) {
    /* Flash thật sau erase = 0xFF */
    memset(FLASH, 0xFF, sizeof(FLASH));
}

void AT45DB_ReadPage(uint16_t page, uint8_t *buf, uint8_t *spare) {
    uint8_t data_read[AT45_PAGE_SIZE];
    if (page >= AT45_TOTAL_PAGES) return;

    uint32_t addr = page * AT45_PAGE_SIZE;
    memcpy(data_read, &FLASH[addr], AT45_PAGE_SIZE);
    
    if(buf)   memcpy(buf, data_read, 512);
    if(spare) memcpy(spare, data_read + 512, 16); 
     
}

void AT45DB_WritePage(uint16_t page, const uint8_t *buf, const uint8_t *spare) {
    uint8_t data_write[AT45_PAGE_SIZE];
    if (page >= AT45_TOTAL_PAGES || buf == NULL)
        return;

    uint32_t addr = page * AT45_PAGE_SIZE;  
    memcpy(data_write, buf, 512);

    if (spare != NULL) {
        memcpy(data_write + 512, spare, 16);
    } else {
        memset(data_write + 512, 0x00, 16);  
    }

    memcpy(&FLASH[addr], data_write, AT45_PAGE_SIZE);
}

void AT45DB_EraseBlock(uint16_t block_id)
{
    if (block_id >= AT45_TOTAL_BLOCKS)
        return;

    uint32_t start_page = (uint32_t)block_id * AT45_PAGE_PER_BLOCK;
    uint32_t start_addr = start_page * AT45_PAGE_SIZE;

    memset(&FLASH[start_addr],
           0xFF,
           AT45_PAGE_PER_BLOCK * AT45_PAGE_SIZE);
}
