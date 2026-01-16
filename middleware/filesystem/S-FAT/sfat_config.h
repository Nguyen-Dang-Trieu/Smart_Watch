#ifndef H_SFAT_CONFIG_H
#define H_SFAT_CONFIG_H

/**
 * Các thông số sfat cơ bản (sector = 512 bytes)
 * - Superblock: 32bytes -> 1 sectotr 
 * - Directory Area: 1024 file entries -> 1024 * 16 bytes = 16384 bytes -> 32 sectors
 * - Fat: Tổng cluster = 4096 (1 cluster = 1 sector)
 *   + Số lượng entry của fat = số lượng cluster có trong filesystem
 *   + Fat entry: 2 bytes -> Tổng Fat size = 4096 * 2 bytes = 8192 bytes -> 16 sectors
 * - Data Blocks: 4096 sectors
 *   + Số sector còn lại = 4096 - (1 + 32 + 16) = 4047 sectors.
 */

#define SFAT_BYTES_PER_SECTOR     512    // Bytes of per sector
#define SFAT_SECTORS_PER_CLUSTER  1      // Sectors of per cluster
#define SFAT_TOTAL_SECTORS        4096
#define SFAT_BYTES_PER_CLUSTER    (SFAT_BYTES_PER_SECTOR * SFAT_SECTORS_PER_CLUSTER)
#define SFAT_TOTAL_CLUSTERS       (SFAT_TOTAL_SECTORS / SFAT_SECTORS_PER_CLUSTER)

/* Superblock */
#define SFAT_START_SUPERBLOCK     0      // Starting sector: sector 0
#define SFAT_SUPERBLOCK_SECTORS   1      // Number of sectors used for SuperBlock

/* Directory */
#define SFAT_START_DIR               1   // Starting sector: sector 1
#define SFAT_DIR_SECTORS             1   // Number of sectors used for Directory
#define SFAT_DIR_ENTRY               16  // Size of each dir entry: 16 Bytes
#define SFAT_MAX_DIRENTRIES          32  // Total number of dir entries
#define SFAT_USED_DIRENTRIES         0   // Total number of dir entries used

/* FAT */
#define SFAT_START_FAT                   33  // Starting sector: sector 33
#define SFAT_FAT_SECTORS                 16  // Number of sectors used for Fat
#define SFAT_FAT_ENTRY                   2   // Size of each fat entry
#define SFAT_FAT_ENTRIES_PER_SECTOR     (SFAT_BYTES_PER_SECTOR / SFAT_FAT_ENTRY) // 256
#define SFAT_MAX_FAT                     4096
#define SFAT_END_CLUSTER                 0x0FFF    
#define SFAT_FREE_CLUSTER                0x0000
#define MAX_ALLOC                        5   // Maximum number of clusters  

/* Data blocks */
#define SFAT_START_DATA           (SFAT_START_FAT + SFAT_FAT_SECTORS)

/* File descriptor */
#define SFAT_MAX_OPEN_FILES       10

/* Debug */
#define SFAT_ENABLE_DEBUG         0

#endif // H_SFAT_CONFIG_H