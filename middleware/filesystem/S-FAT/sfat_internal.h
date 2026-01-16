#ifndef H_SFAT_INTERNAL_H
#define H_SFAT_INTERNAL_H

/**
 * Các hàm nội bộ của SFAT filesystem, không được sử dụng bên ngoài module này
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sfat_config.h"
#include "FTL_map.h"

// Helper macros
#define INDEX_OF_SAFE(ptr, arr_base, arr_size)                                 \
({                                                                             \
    ptrdiff_t __idx = (ptr) - (arr_base);                                      \
    (__idx >= 0 && (size_t)__idx < (arr_size)) ? (size_t)__idx : (size_t)(-1); \
})

#define CALC_CLUSTERS(data_sz)     (((data_sz) + (SFAT_BYTES_PER_CLUSTER) - 1) / (SFAT_BYTES_PER_CLUSTER))

/* Macro lấy chỉ số Sector chứa Cluster tương ứng trong bảng FAT */
#define GET_FAT_SECTOR(cluster)    ((cluster) / SFAT_FAT_ENTRIES_PER_SECTOR) 

/* Macro lấy vị trí (offset) của Cluster trong một Sector FAT */
#define GET_FAT_OFFSET(cluster)    ((cluster) % SFAT_FAT_ENTRIES_PER_SECTOR) 

typedef struct SuperBlock
{
  uint16_t total_sectors;         // Total number of sectors
  uint16_t sectors_per_cluster;   // Number of sectors in each block
  uint16_t bytes_per_sector;      // Number of bytes in each sector
  uint16_t available_sectors;     // Number of sectors available
  uint16_t total_direntries;      // Total number of directory entries (files)
  uint16_t available_direntries;  // Number of available dir entries
  uint8_t  fs_type;               // File system type (FA for SFAT)
  uint8_t  reserved[11];          // Reserved, all set to 0
  uint8_t  label[20];             // Not guaranteed to be NUL-terminated
}SuperBlock_t;


typedef struct __attribute__((packed))
{
  uint8_t  name[10];     // Not guaranteed to be NUL-terminated
  int16_t  fat_entry;    // First data block
  uint32_t size;         // Size of the file in bytes
} DirEntry_t;

typedef struct FileDescriptor {
  DirEntry_t *dir_entry;  // Trỏ tới dir entry của file.
  uint32_t    offset;     // vị trí đọc/ghi hiện tại của file.
  uint8_t     mode;       // Chế độ hoạt động khi write
  bool        used;       // Cờ đánh dấu file descriptor đang được sử dụng hay không.
} FD_t;

extern SuperBlock_t superblock;

typedef struct {
  bool isInitialized;
  SuperBlock_t superblock;
} singleton_sfat_t;



/**
 * @brief Hàm này tìm kiếm metadata của file trong bộ đệm recently_opened_files
 * @param recently_opened_files Mảng chứa metadata của các file đã mở gần đây
 * @param filename Tên file cần tìm kiếm
 * @retval Vị trí metadata trong recently_opened_files nếu tìm thấy, ngược lại trả về -1
 */
int8_t get_metadata_from_cache(DirEntry_t *recently_opened_files, const char *filename);

/**
 * @brief Hàm này tìm kiếm metadata của file trong directory area
 */
int8_t get_metadata_from_dir(FD_t *fd_table, DirEntry_t *recently_opened_files, const char *filename);

/**
 * @brief Hàm này tìm vị trí file descriptor trống trong fd_table
 * @param fd_table Bảng file descriptor
 * @retval Vị trí fd trống nếu tìm thấy, ngược lại trả về -1
 */
int8_t find_free_fd(FD_t *fd_table);

void sfat_read_sector(uint16_t sector, uint8_t *buf);
void sfat_write_sector(uint16_t sector, const uint8_t *buf);

#endif // H_SFAT_INTERNAL_H