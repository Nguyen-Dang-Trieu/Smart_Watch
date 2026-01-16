#ifndef H_DEBUG_SFAT_H
#define H_DEBUG_SFAT_H

#include <stdio.h>
#include <ctype.h>
#include "sfat.h"
#include "flash.h"

#define ADDRESS_DUMP_START_DIR_ENTRY 0x0000
#define SECTOR_SIZE                  (superblock.bytes_per_sector)

// Địa chỉ hiện tại các khu vực trong flash memory 
#define ADDRESS_SUPERBLOCK     0
#define ADDRESS_DIRECTORY_AREA (ADDRESS_SUPERBLOCK + 1 * SFAT_BYTES_PER_SECTOR)
#define ADDRESS_FAT_AREA       (ADDRESS_DIRECTORY_AREA + 32 * SFAT_BYTES_PER_SECTOR)
#define ADDRESS_DATA_BLOCKS    (ADDRESS_FAT_AREA + 16 * SFAT_BYTES_PER_SECTOR)


void sfat_dump_superblock();

#define FAT_SIZE (4096 * 2) // 4096 entries, 2 bytes each

// ------------- DUMP FAT ---------------

/**
 * @brief Dump toàn bộ bảng FAT
 */
void sfat_dump_fat(void);

/**
 * @brief Dump một phạm vi trong bảng FAT
 * @param start_offset Vị trí bắt đầu
 * @param end_offset Vị trí kết thúc
 * @note Phạm vi: [start_offset, end_offset] = [0, 4095]
 */
void sfat_dump_fat_range(size_t start_cluster, size_t end_cluster);

/**
 * @brief Dump nội dung của một file entry trong dictory
 * @param index Chỉ mục của file entry.
 */
void sfat_dump_directory_entry(int index); 

/**
 * @brief Dump toàn bộ directory entries
 */
void sfat_dump_directory(void);

void sfat_dump_cluster(uint16_t cl);

/**
 * @brief Dump nội dung của một sector được chỉ định tại địa chỉ cụ thể
 * @param addr Vị trí của sector
 */
void sfat_dump_sector(uint32_t sector_index);

#endif // H_DEBUG_SFAT_H