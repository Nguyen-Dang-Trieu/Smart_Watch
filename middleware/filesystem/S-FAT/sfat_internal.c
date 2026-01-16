#include "sfat_internal.h"

/**
 * @brief Hàm này tính MEX (Minimum EXcluded value) của mảng `arr`
 * @param arr Mảng các số nguyên
 * @param size_arr Kích thước của mảng arr
 * @retval Giá trị MEX tìm được
 * @note Nếu tất cả các vị trí từ 0 đến size_arr - 1 đều được sử dụng, hàm sẽ trả về SFAT_MAX_OPEN_FILES
 */
static int8_t  mex(int8_t *arr, size_t size_arr)
{
    bool used[SFAT_MAX_OPEN_FILES];
    memset(used, false, sizeof(used));

    // mark the index that are used
    for (uint8_t i = 0; i < SFAT_MAX_OPEN_FILES; i++)
    {
        if (arr[i] >= 0 && arr[i] < SFAT_MAX_OPEN_FILES)
            used[arr[i]] = true;
    }

    // find the mex
    uint8_t result = 0;
    while (used[result] && result < size_arr)
    {
        result++;
    }

    if (result == size_arr)
    {
        return -1; // all slots are used
    }

    return result;
}

int8_t get_metadata_from_cache(DirEntry_t *recently_opened_files, const char *filename)
{
    int8_t metadata_pos = -1;
    for (uint8_t i = 0; i < SFAT_MAX_OPEN_FILES; i++)
    {
        if (strncmp((const char *)recently_opened_files[i].name, filename, 10) == 0)
        {
            metadata_pos = i;
            break;
        }
    }

    return metadata_pos;
}

int8_t get_metadata_from_dir(FD_t *fd_table, DirEntry_t *recently_opened_files, const char *filename)
{
    int8_t      metadata_pos  = -1;               
    uint8_t dir_buf[SFAT_BYTES_PER_SECTOR];
    DirEntry_t *entries = (DirEntry_t *)dir_buf;
    sfat_read_sector(SFAT_START_DIR, dir_buf);

    for (uint8_t i = 0; i < SFAT_MAX_DIRENTRIES; i++)
    {
        if (strncmp((const char *)entries[i].name, filename, 10) == 0)
        {
            int8_t is_slot_busy[SFAT_MAX_OPEN_FILES]; // Chứa các vị trí metadata đang được fd sử dụng
            memset(is_slot_busy, -1, sizeof(is_slot_busy));

            // Tìm tất cả các vị trí metadata đang được sử dụng
            for (uint8_t j = 0; j < SFAT_MAX_OPEN_FILES; j++)
            {
                if (fd_table[j].used == true)
                {
                    is_slot_busy[j] = INDEX_OF_SAFE(fd_table[j].dir_entry, recently_opened_files, SFAT_MAX_OPEN_FILES);
                }
            }

            // Tìm vị trí metadata trống 
            metadata_pos = mex(is_slot_busy, SFAT_MAX_OPEN_FILES);
            if (metadata_pos == -1)
            {
                // SFAT_ERR_NOMEM = -6
                metadata_pos = -6; // Không còn slot metadata trống
            }

            // Copy thông tin file vào recently_opened_files
            memcpy(&recently_opened_files[metadata_pos], &entries[i], sizeof(DirEntry_t));
        }
    }

    return metadata_pos;
}

int8_t find_free_fd(FD_t *fd_table)
{
    for(int8_t i = 0; i < SFAT_MAX_OPEN_FILES; i++) { 
        if(fd_table[i].used == false) { 
            return i;
        }
    }

    return -1;
}

/**
 * @brief Đọc 1 sector từ Flash
 * @param sector Vị trí của sector
 * @param buf Buffer chứa dữ liệu đọc được từ sector
 * 
 */
void sfat_read_sector(uint16_t sector, uint8_t *buf) {
    FTL_Read(sector, buf);
}

void sfat_write_sector(uint16_t sector, const uint8_t *buf) {
    FTL_Write(sector, buf);
}