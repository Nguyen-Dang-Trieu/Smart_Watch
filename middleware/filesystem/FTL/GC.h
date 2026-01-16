#ifndef H_GC_H
#define H_GC_H

#include "policy.h"

typedef struct Flash_Translation_Layer FTL_t;
#define GC_TRIGGER    10 // số lần ghi ftl_write để kích hoạt GC

typedef struct {
    uint16_t victim_block;  // block hiện tại đang được GC xử lí
    uint8_t  write_count;   // Số lần ghi ftl_write để kích hoạt GC
    bool     active;        // Kích hoạt GC 
} GCContext_t;

extern GCContext_t GC_Module;

void GC_Init( GCContext_t *gc );

/**
 * @brief Chạy Garbage collection
 * @param GC trỏ đến tối tượng quản li
 */
void GC_Run( GCContext_t *gc, FTL_t *ftl); 

/**
 * @brief Tăng biến đếm write_count
 */
void GC_OnWrite( GCContext_t * gc);
; 

// Đọc nội dung của page còn sống và đưa vào buffer
void GC_ReadValidPage( uint16_t block_id, uint8_t page, uint8_t *data_buf, uint8_t *spare_buf); 

// Cấp phát page trống mới và ghi dữ liệu đã đọc và page mới
void GC_WriteNewPage( FTL_t *ftl, uint16_t lp,  uint8_t *data_buf);

// Xóa hoàn toàn victim block sau khi copy xong, reset metadata block, tăng erase_count
void GC_EraseBlocK( FTL_t* ftl, uint16_t id_of_victimblock ); 

void GC_UpdateBlock(uint16_t block_id, FTL_t *ftl);

/**
 * @brief Hàm in các blocks hiện tại đang chờ được xử lí trong GC
 * @param GC trỏ đến đối tượng quản lí
 */
void GC_PrintBlocks(void); 


#endif // H_GC_H