#ifndef H_DEBUG_FTL_H
#define H_DEBUG_FTL_H

#include <stdio.h>
#include <stdint.h>
#include "FTL.h"

void FTL_DumpMapping( uint16_t from, uint16_t to );
void FTL_DumpBlockInfo( uint16_t from, uint16_t to );
void FTL_DumpBlockPages( uint16_t block );

/**
 * @brief In ra nội dung của logical page
 * @param logical_page
 */
void FTL_DumpLogicalPage( uint16_t logical_page );

/**
 * @brief Hiển thị trạng thái bên trong lru cache
 */
void FTL_PrintCache(void);

void dump_flash_page( uint16_t physical_page );

/**
 * @brief Hiển thị thông tin của toàn bộ physical page chứa dữ liệu mapping
 */
void dump_mapping_page(uint32_t phy_page);
#endif // H_DEBUG_FTL_H