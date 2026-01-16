#include "PVB.h"

int main(void) {
    PVB_t pvb = {0}; // Khởi tạo PVB rỗng

    // Thêm một vài block vào memtable
    Put_PVB(&pvb, (Block_SL_t){.BlockID=10, .bitMap=0x01, .erase_flag=false});
    Put_PVB(&pvb, (Block_SL_t){.BlockID=5,  .bitMap=0x02, .erase_flag=false});
    Put_PVB(&pvb, (Block_SL_t){.BlockID=7,  .bitMap=0x04, .erase_flag=false});
    Put_PVB(&pvb, (Block_SL_t){.BlockID=10, .bitMap=0x08, .erase_flag=false}); // update block 10
    Put_PVB(&pvb, (Block_SL_t){.BlockID=3,  .bitMap=0x01, .erase_flag=false}); 

    // In trạng thái sau khi flush
    printMemtable(&pvb); // memtable đã reset về size=0
    printL0(&pvb);       // L0 chứa dữ liệu đã merge

    Put_PVB(&pvb, (Block_SL_t){.BlockID=6,  .bitMap=0x01, .erase_flag=false}); // -> Full 5 memtable xuống L0
    printMemtable(&pvb); // memtable đã reset về size=0
    printL0(&pvb);       // L0 chứa dữ liệu đã merge

    // Thêm block đã bị GC
    Put_PVB(&pvb, (Block_SL_t){.BlockID=7, .bitMap=0, .erase_flag=true});
    printMemtable(&pvb);

    // Lấy trạng thái block 10
    Block_SL_t b10 = Get_PVB(&pvb, 10);
    printf("\nGet block 10: ID=%d, bitMap=%d, erase_flag=%d\n",
           b10.BlockID, b10.bitMap, b10.erase_flag);

    // Lấy trạng thái block 7 (đã GC)
    Block_SL_t b7 = Get_PVB(&pvb, 7);
    printf("Get block 7: ID=%d, bitMap=%d, erase_flag=%d\n",
           b7.BlockID, b7.bitMap, b7.erase_flag);

    return 0;
}
