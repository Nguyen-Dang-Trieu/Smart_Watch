#ifndef H_PVB_H
#define H_PVB_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MEMTABLE_MAX_SIZE   5
#define L0_MAX_SIZE         512 // L0 phải chứa tất cả các block mà FTL dùng

#define PAGE_STATE_FREE      0x0   // page chưa ghi
#define PAGE_STATE_VALID     0x1   // page đang chứa data hợp lệ
#define PAGE_STATE_INVALID   0x3   // page rác 

#define GET_PAGE_STATE(bm, i) (((bm) >> ((i)*2)) & 0x3)
#define SET_PAGE_STATE(bm, i, s) \
    ((bm) = ((bm) & ~(0x3 << ((i)*2))) | ((s) << ((i)*2)))

typedef union {
  uint16_t raw;
  struct {
    uint16_t page_0: 2;
    uint16_t page_1: 2;
    uint16_t page_2: 2;
    uint16_t page_3: 2;
    uint16_t page_4: 2;
    uint16_t page_5: 2;
    uint16_t page_6: 2;
    uint16_t page_7: 2;
  } pages;
} Bitmap_t;


// Nhật kí thay đổi state của block
typedef struct Block_State_Log{ 
  uint16_t  BlockID;
  Bitmap_t  bitMap;
  bool      erase_flag;
} Block_SL_t;


typedef struct Page_Validity_BitMap {
  Block_SL_t memtable[MEMTABLE_MAX_SIZE];
  Block_SL_t L0[L0_MAX_SIZE];
  size_t     memtable_size; 
  size_t     L0_size;
} PVB_t;

//Thêm hàm
void Init_PVB( void );


/**
 * @brief Thêm nhật kí block vào trong memtable
 * @param pvb 
 * @param entry Entry cần thêm 
 */
void Put_PVB( PVB_t *pvb, Block_SL_t block );

/**
 * @brief Trả về trạng thái page của một block theo ID
 * @param pvb
 * @param id ID của block cần tìm
 * @retval Trạng thái hiện tại của block
 */
Block_SL_t Get_PVB( PVB_t *pvb, uint16_t id );


// Debug API
void printMemtable( PVB_t *pvb );
void printL0( PVB_t *pvb );


#endif // H_PVB_H