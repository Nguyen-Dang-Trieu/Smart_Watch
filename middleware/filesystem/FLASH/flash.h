#ifndef H_HW_H
#define H_HW_H

#include <stdint.h>
#include <string.h>
#include "sfat_config.h"

#define AT45_PAGE_SIZE          528
#define AT45_TOTAL_PAGES        4096
#define AT45_PAGE_PER_BLOCK     8
#define AT45_TOTAL_BLOCKS       (AT45_TOTAL_PAGES / AT45_PAGE_PER_BLOCK)

extern uint8_t FLASH[AT45_TOTAL_PAGES * AT45_PAGE_SIZE];

void AT45DB_Init(void);

/* Page-level API */

/**
 * @brief Đọc một page
 * @param page Chỉ số physical page
 * @param buf  Buffer chứa dữ liệu page
 * @param spare Chứa thông tin của block
 */
void AT45DB_ReadPage(uint16_t page, uint8_t *buf, uint8_t *spare);

/**
 * @brief Ghi một page
 * @param page Chỉ số physical page
 * @param buf  Buffer chứa dữ liệu page
 * @param spare Chứa thông tin của block
 */
void AT45DB_WritePage(uint16_t page, const uint8_t *buf, const uint8_t *spare);

/* Block-level API (chuẩn flash) */
void AT45DB_EraseBlock(uint16_t block_id);

#endif // H_HW_H