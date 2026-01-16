#ifndef H_SFAT_H
#define H_SFAT_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "sfat_config.h"
#include "sfat_internal.h"
#include "flash.h"
#include "FTL_map.h"

#ifdef SFAT_ENABLE_DEBUG
#define SFAT_LOG(fmt, ...) printf("[SFAT] " fmt "\n", ##__VA_ARGS__)
#else
#define SFAT_LOG(fmt, ...)
#endif


#define SFAT_CLUSTERS_FOR_SIZE(size)  (((size) + SFAT_BYTES_PER_CLUSTER - 1) / SFAT_BYTES_PER_CLUSTER)

/* SFAT Error Codes (Based on POSIX errno style) */
#define SFAT_OK              0   // thành công
#define SFAT_ERR_BAD_FD     -1   // file descriptor không hợp lệ
#define SFAT_ERR_NOT_OPEN   -2   // file chưa mở
#define SFAT_ERR_BAD_MODE   -3   // mode không hợp lệ cho hành động
#define SFAT_ERR_NULL_PTR   -4   // buffer NULL
#define SFAT_ERR_NOENT      -5   // file không tồn tại
#define SFAT_ERR_NOMEM      -6   // không còn slot fd trống, không còn entry metadata trống
#define SFAT_ERR_NODATA     -7   // không đủ sector / cluster, không đủ chỗ tạo file
#define SFAT_ERR_EOF        -8   // đã đọc tới EOF

/* Operating mode */
#define SFAT_NO_MODE      0x00  // N/A
#define SFAT_MODE_W       0x01  // write
#define SFAT_MODE_R       0x02  // Read
#define SFAT_MODE_A       0x03  // Append
#define SFAT_MODE_RP      0x04  // read + write ("r+")

// ------ API cho SFAT Filesystem ------

/**
 * @brief Khởi động SFAT filesystem, đọc thông tin từ flash memory vào RAMs
 * @param none
 */
void sfat_mount(void);

/**
 * @brief Lưu thông tin filesystem từ RAMs về flash memory
 * @param none
 */
void sfat_umount(void);


/**
 * @brief In ra các thông in trong superblock
 * @param none
 */
void sfat_state(void);


// ------ API cho thao tác file trong SFAT Filesystem ------

/**
 * @brief Tạo file
 * @param name Tên file 
 * @retval SFAT_OK - Tạo file thành công
 * @retval SFAT_ERR_NODATA - Thất bại
 */
int8_t sfat_create(const char *name);

/**
 * @brief Xóa file
 * @param name Tên file cần xóa
 * @retval     SFAT_OK        File được tạo thành công
 * @retval     SFAT_ERR_NODATA Không còn entry trống, không thể tạo file
 */
int8_t sfat_delete(const char *name);


/**
 * @brief Mở file 
 * @param filename Tên file 
 * @param mode Chế độ hoạt động khi mở file ("w", "a", "r+")
 * @param offset Offset ban đầu khi ghi (chỉ dùng khi mode "r+")
 * @retval fd của file vừa mở. 
 * @note Hàm này sẽ tìm file trong Directory Area, nếu file không tồn tại thì trả về error.
 */
int8_t sfat_open(const char *filename, uint8_t mode);
/**
 * @brief Ghi dữ liệu vào file
 * @param fd File descriptor của file
 * @param buf Con trỏ tới buffer chứa dữ liệu
 * @param size Kích thước dữ liệu (tính bằng bytes)
 * @retval none
 */
int8_t sfat_write(int fd, const void *buf, uint32_t size);

/**
 * @brief Đọc dữ liệu từ file
 * @param fd File descriptor
 * @param buf Buffer lưu trữ
 * @param size Số bytes cần đọc
 * @retval Số byte thực sự đọc được
 */
uint32_t sfat_read(uint8_t fd, void *buf, uint32_t size); 

/**
 * @brief Xóa fd của file đang mở
 * @param fd File description của file.
 * @retval none
 */
void sfat_close(uint8_t fd);

/**
 * @brief Lấy thông tin file từ tên Directory Area
 * 
 */
int sfat_stat(const char *name); 




#endif // H_SFAT_H