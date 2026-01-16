#ifndef H_FTL_H
#define H_FTL_H

#include <stdint.h>
#include <stdbool.h>


/* --- 1. INCLUDE CÁC MODULE  --- */
#include "flash.h"
#include "cache.h"
#include "lru.h"
#include "PVB.h"
#include "GC.h"

/* --- 2. CẤU HÌNH HỆ THỐNG (HARDWARE/LAYOUT) --- */
#define PAGE_SIZE               528         // Dùng Flash AT45DB161 mặc định 528 bytes/page
#define PAGES_PER_BLOCK         8
#define TOTAL_PHYSICAL_PAGES    4096
#define TOTAL_LOGICAL_PAGES     4096
#define TOTAL_BLOCKS            (TOTAL_PHYSICAL_PAGES / PAGES_PER_BLOCK) // Hiện tại là 512 block

/* Quản lý Mapping Page */
#define PAGES_PER_RANGE        128
#define TOTAL_RANGES           (TOTAL_LOGICAL_PAGES / PAGES_PER_RANGE)
#define START_METADATA          0  
#define BLOCKS_OF_METADATA      12 // Tổng số blocks dành cho metadata FTL
#define MAP_MAGIC               0x4D50 // 'MP'

/* Quản lý vùng ghi dữ liệu */
#define NUM_TEST_BLOCKS        10 // Số lượng block data đang test - Chỉ sử dụng khi test tính năng FTL
#define BLOCK_START_WRITE      TOTAL_BLOCKS - NUM_TEST_BLOCKS // Block bắt đầu ghi dữ liệu

/* Trạng thái lỗi/Trống */
#define INVALID_PAGE            0xFFFF
#define INVALID_BLOCK           0xFFFF

/* --- 3. MACROS HỖ TRỢ (UTILITIES) --- */
#define PAGE_TO_BLOCK(p)        ((p) / PAGES_PER_BLOCK) // Xác định page thuộc block nào?
#define PAGE_OFFSET_IN_BLOCK(p) ((p) % PAGES_PER_BLOCK) // Xác định page nằm ở vị trí nào trong block
#define BLOCK_TO_PAGE(b)        ((b) * PAGES_PER_BLOCK) // Tìm vị trí page đầu tiên của block


/* --- 4. ĐỊNH NGHĨA KIỂU DỮ LIỆU (DATA STRUCTURES) --- */

/**
 * @brief Cấu trúc dữ liệu dự phòng (Spare Area) trên mỗi Page
 */
typedef union {
  uint8_t  raw[16]; // Tổng 16 bytes
  struct __attribute__((packed)) {
    uint16_t logical_page;
    uint8_t  reserved[14]; // Dành cho những tính năng sau
  } fields;
} spare_t;

/**
 * @brief Cấu trúc trang Mapping lưu trữ trên Flash
 */
typedef struct __attribute__((packed)) {
    uint16_t magic;                    // Định danh mapping page
    uint16_t range_index;              // Chỉ số range (0 -> TOTAL_RANGES - 1)
    uint16_t entries[PAGES_PER_RANGE]; // L2P Map: entries[LP] = PP
} mapping_page_t;

/**
 * Tốt nhất là erase_count sẽ là 18bits -> 2^18 = 262 144 > 100 000 số lần ghi/xóa của flash
 * free_pages và invalid_pages là 3 bit, số lượng 8 là đủ
 * 
 * Chắc là phải thêm 1 trườn nữa là invalid_pages
 */

 /**
 * @brief Quản lý trạng thái từng Block vật lý
 */
typedef struct Block_State{
    uint16_t erase_count;   // Số lần xóa của block
    uint8_t  free_pages;    // Số lượng page free
    uint8_t  invalid_pages; // Số lượng page rác
} Block_State_t;

/**
 * @brief Thông tin Mapping Range lưu trên RAM
 */
typedef struct {
    uint16_t physical_page; // // Vị trí PP trên flash đang giữ mapping table của range này
} Logical_range_t;

/**
 * @brief Đối tượng quản lý chính (FTL Context)
 */
typedef struct Flash_Translation_Layer {
    /* Quản lí các logical range nằm trên RAM */
    Logical_range_t range_map[TOTAL_RANGES]; 

    /* Quản lí state các blocks có trong hệ thống */
    Block_State_t BlockAllocationTable[TOTAL_BLOCKS];

    /* Bảng Page Validity BitMap -> Trạng thái từng page trong block đã từng sử dụng */
    PVB_t pvb;

    /* Garbage Collection */
    GCContext_t *gc;

    /* Con trỏ ghi hiện tại (Active Block/Page) */
    struct {
        uint16_t block_idx;         // Block đang dùng để ghi dữ liệu mới
        uint16_t page_idx;          // Page tiếp theo sẽ ghi trong block đó
    } curr_write;

} FTL_t;

/* ---------------------- GLOBAL EXTERN ---------------------- */
extern FTL_t   ftl;
extern Cache_t FTL_cache; // Hiện tại LRU cache chỉ hỗ trợ cho FTL_Read
extern LRU_t   LRU;

#define DEBUG_FTL   // Enable Debug

#ifdef DEBUG_FTL
    extern uint16_t l2p[TOTAL_LOGICAL_PAGES];
#endif


/* ---------------------- PUBLIC APIs --------------------- */

/** @section Core Functions */
void FTL_Init( );
void FTL_Read( uint16_t logical_page, uint8_t* buffer );
void FTL_Write( uint16_t logical_page, const uint8_t* buffer );


/** @section Helpers functions */
/**
 * @brief Tìm block free trong Block Allocation Table 
 * 
 * Block free chỉ khi block.free_pages > 0
 */
int16_t _Find_free_block( FTL_t*ftl, Block_State_t *work_area );

/**
 * @brief Tìm vị trí page free trong block
 * @retval 
 * 
 * - (-1) nếu không có page nào hiện dang trống. 
 * 
 * - Có page free thì sẽ dc giá trị từ  0 -> 7
 */
int8_t _Find_free_page( uint16_t block_id, PVB_t *pvb );

/**
 * @brief Update metadata của mapping page ở trong flash
 * @param range_map Mảng các vị trí range chứa trong physical page trên flash
 * @param lp logical page
 * @param pp physical page
 * @retval
 */
void _Update_mapping_page( uint16_t lp, uint16_t pp ); 

/** @section Debug & Maintenance */
void FTL_PrintGC(void);
void cachePrint(void); // Được gọi từ thư viện debug_ftl

#endif // H_FTL_MAP_H