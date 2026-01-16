#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "FTL.h"
#include "debug_ftl.h"

// Hiện tại chỉ test trên 20 block cuối cùng
#define BLOCK_WRITE 4016 // Dữ liệu được lưu bắt đầu từ page physical thứ 4016

static uint8_t buf_write[PAGE_SIZE];
static uint8_t buf_read[PAGE_SIZE];

void fill_buffer(uint8_t v)
{
    memset(buf_write, v, PAGE_SIZE);
}

void test_All_FTL_Features(void) {
    fill_buffer('1');

    
    for(uint8_t i = 0; i < 9; i++) { // Ghi tới lần thứ 9 của logical 0
        FTL_Write(0, buf_write);
    }
    

    FTL_Write(1, buf_write); 
    FTL_Write(2, buf_write); 
    FTL_Write(3, buf_write); 
    FTL_Write(4, buf_write); 
    FTL_Write(5, buf_write); 
    FTL_Write(6, buf_write); 
    FTL_Write(7, buf_write); 
    FTL_Write(8, buf_write); 


    printMemtable(&ftl.pvb);
    printL0(&ftl.pvb);
    
    // dump_mapping_page(0); range index = 0 -> quản li logical: 0 -> 127
    FTL_PrintGC();

    // FTL_DumpBlockInfo(502, 512);
    
}


int main(void)
{
    AT45DB_Init();
    FTL_Init();

    test_All_FTL_Features();

    return 0;
}