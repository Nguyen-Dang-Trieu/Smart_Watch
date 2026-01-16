#ifndef H_POLICY_H
#define H_POLICY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_HEAP                512 // Tổng số block mà FTL dùng
#define GET_PARENT(i)           ((i - 1) / 2)
#define GET_LEFT_CHILD(i)       ((2 * i) + 1)
#define GET_RIGHT_CHILD(i)      ((2 * i) + 2)
#define NOT_IN_HEAP             0xFFFE
#define EMPTY_HEAP              0xFFFD
#define WEAR_SHIFT              5 // Viết document để giải thích 

typedef struct {
    uint16_t block_id;
    uint16_t cost;
} HeapNode_t;

/**
 * @brief Khởi tạo Heap và pos_in_heap
 */
void Policy_Init( void );

/**
 * @brief Update or insert block
 * @note Mỗi khi thông tin của block đó thay đổi dựa trên valid_pages, erase_count, đều phải gọi hàm này.
 * 
 * - valid_pages = PAGES_PER_BLOCK - invalid_pages - free_pages.
 */
void Policy_UpdateOrInsert( uint16_t block_id, uint16_t valid_pages, uint16_t erase_count );

/**
 * @brief Lấy victim block
 * @retval ID của victim block
 */
uint16_t Policy_GetVictim( void );

/**
 * @brief Gọi sau khi block được xóa
 */
void Policy_OnErase(uint16_t block_id, uint16_t new_erase_count);


void Policy_PrintHeap(void);
#endif // H_VICTIM_BLOCK_H