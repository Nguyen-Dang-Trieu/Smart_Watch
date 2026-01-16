#include "FTL.h"

/** Debug */
#ifdef DEBUG_FTL
    uint16_t l2p[TOTAL_LOGICAL_PAGES];
#endif

FTL_t ftl;

// The current cache is reserved for FTL.
Cache_t FTL_cache; // Hiện tại LRU cache chỉ hỗ trợ cho FTL_Read
LRU_t   LRU;


/* --- Private Function Prototypes (Internal Use Only) ---------------------------------------------- */

static void _Init_RangeMap(Logical_range_t* map_table);
static void _Init_BlockAllocationTable( Block_State_t *table );
static void _Init_PVB( PVB_t *pvb );
static void _Init_CurrWrite( FTL_t *ftl );

static void _Invalidate_old_physical_page(uint16_t lp);


/* --- Define Private Functions ---------------------------------- */
static void _Init_RangeMap(Logical_range_t* map_table) 
{
    // Khởi tạo mapping page cho tửng range
    for(uint8_t _range = 0; _range < TOTAL_RANGES; _range++) {
        uint8_t data_buf[512];
        uint8_t spare_buf[16];

        // State init của mapping page
        mapping_page_t mp;
        mp.magic       = MAP_MAGIC;
        mp.range_index = _range;

        for (uint16_t i = 0; i < PAGES_PER_RANGE; i++) {
            mp.entries[i] = INVALID_PAGE;
        }

        // Địa chỉ physical để lưu bảng mapping
        uint16_t physical = _range + START_METADATA; 

        map_table[_range].physical_page = physical;

        memset(data_buf, 0x00, 512);       // 0x00 biểu thị byte đó chưa được sử dụng
        memcpy(data_buf, &mp, sizeof(mapping_page_t));

        memset(spare_buf, 0x00, 16);       // 0x00 biểu thị byte đó chưa được sử dụng
        memcpy(spare_buf, &mp, sizeof(spare_t));

        AT45DB_WritePage(physical, data_buf, spare_buf);
    }
}

static void _Init_BlockAllocationTable( Block_State_t *table ) 
{
    for (uint16_t b = 0; b < TOTAL_BLOCKS; b++) {
        table[b].free_pages  = PAGES_PER_BLOCK;
        table[b].erase_count = 0;
    }
}

static void _Init_PVB( PVB_t *pvb )
{
    memset(pvb, 0, sizeof(PVB_t));
}

static void _Init_GC( void)
{
    ftl.gc = &GC_Module;
    GC_Init(ftl.gc);
}

static void _Init_CurrWrite( FTL_t *ftl ) 
{
    ftl->curr_write.block_idx = BLOCK_START_WRITE;
    ftl->curr_write.page_idx  = 0;  // Luôn bắt đầu từ page 0 của block khi hệ thống lần đầu chạy
}

int16_t _Find_free_block(FTL_t *ftl, Block_State_t *work_area) {
    for (uint16_t i = 0; i < TOTAL_BLOCKS; i++) {
        uint16_t b = (ftl->curr_write.block_idx + i) % TOTAL_BLOCKS;

        if (b < BLOCK_START_WRITE) continue; // Tránh vùng metadata
        
        if (work_area[b].free_pages > 0) {
            // Lưu lại block tìm được vào block_idx 
            ftl->curr_write.block_idx = b; 
            return b;
        }
    }
    return -1; // Flash Full -> Cần gọi Garbage Collection
}

int8_t _Find_free_page(uint16_t block_id, PVB_t *pvb)
{
    Block_SL_t block_temp = Get_PVB(pvb, block_id);

    if (block_temp.erase_flag == true) return 0;

    for (uint8_t i = 0; i < PAGES_PER_BLOCK; i++) {
        if(GET_PAGE_STATE(block_temp.bitMap.raw, i) == PAGE_STATE_FREE) {
            return i;
        }
    }

    return -1; // Block full
}

void _Update_mapping_page(uint16_t lp, uint16_t pp )
{
    uint8_t data_buf[512];
    uint16_t range_index = lp / PAGES_PER_RANGE;
    uint16_t offset      = lp % PAGES_PER_RANGE;

    mapping_page_t mp;

    uint16_t mapping_page = ftl.range_map[range_index].physical_page; // Lấy địa chỉ của page vật lí

    AT45DB_ReadPage(mapping_page, data_buf, NULL);
    memcpy(&mp, data_buf, sizeof(mapping_page_t));

    mp.entries[offset] = pp;

    memcpy(data_buf, &mp, sizeof(mapping_page_t));
    AT45DB_WritePage(mapping_page, data_buf, NULL);
}

static void _Invalidate_old_physical_page(uint16_t lp)
{
    uint8_t buf_page[512];
    mapping_page_t mp;
    
    uint16_t old_physical = INVALID_PAGE;
    // range_index  = lp / PAGES_PER_RANGE
    uint16_t mapping_page = ftl.range_map[lp / PAGES_PER_RANGE].physical_page;

    AT45DB_ReadPage(mapping_page, buf_page, NULL);
    memcpy(&mp, buf_page, sizeof(mapping_page_t));

    // offset = lp % PAGES_PER_RANGE;
    old_physical = mp.entries[lp % PAGES_PER_RANGE];

    if (old_physical != INVALID_PAGE) { // page đã từng được ghi rồi
        uint16_t old_block = PAGE_TO_BLOCK(old_physical);
        uint8_t  old_page  = PAGE_OFFSET_IN_BLOCK(old_physical);

        Block_SL_t inv_log;
        inv_log.BlockID    = old_block;
        inv_log.bitMap.raw = 0;
        SET_PAGE_STATE(inv_log.bitMap.raw, old_page, PAGE_STATE_INVALID);
        inv_log.erase_flag = false;

        Put_PVB(&ftl.pvb, inv_log);
        ftl.BlockAllocationTable[old_block].invalid_pages++;
    }
}

/* --- External Function Definitions (Public Interface) ---------------------------------------------- */

void FTL_Init(void) {
    _Init_RangeMap(ftl.range_map);
    _Init_BlockAllocationTable(ftl.BlockAllocationTable);
    _Init_PVB(&ftl.pvb);
    _Init_GC();
    _Init_CurrWrite(&ftl);

    // Khởi tạo Cache cho system
    LRU_Init(&LRU);
    Cache_init(&FTL_cache, &LRU, &LRU_ops);

#ifdef DEBUG_FTL
    for (uint16_t i = 0; i < TOTAL_LOGICAL_PAGES; i++) {
        l2p[i] = INVALID_PAGE;
    }
#endif
}

void FTL_Read( uint16_t logical_page, uint8_t* buffer ) {
    uint16_t physical_page_to_read = INVALID_PAGE;

    /* Importan: Luôn kiểm tra các tham số đầu vào */
    if (logical_page >= TOTAL_LOGICAL_PAGES || !buffer) {
        if(buffer) // Logical page > TOTAL_LOGICAL_PAGES và buffer không NULL
        {
            memset(buffer, 0xFF, PAGE_SIZE); 
        } 
        return;
    }

    /* Read từ cache */
    int physical_page_temp; 
    if(Cache_Get(&FTL_cache, logical_page, &physical_page_temp) == 1) {
        physical_page_to_read = physical_page_temp;
    }

    /* Read từ flash */
    if(physical_page_to_read == INVALID_PAGE)
    {
        /* 1. Tìm logical page nằm trong range nào */
        uint16_t range_index  = logical_page / PAGES_PER_RANGE;
        uint16_t mapping_page = ftl.range_map[range_index].physical_page; // Vị trí của mapping page trong flash

        /* 2. Đọc bảng mapping từ flash */
        uint8_t data_buf[512];
        mapping_page_t mp;

        AT45DB_ReadPage(mapping_page, data_buf, NULL);
        memcpy(&mp, data_buf, sizeof(mapping_page_t));

        /* 3. Tìm physical page */
        uint16_t offset       = logical_page % PAGES_PER_RANGE; // Vị trí của logical page trong mapping 
        physical_page_to_read = mp.entries[offset];

        /* 4. Update vào Cache */
        Cache_Put(&FTL_cache, logical_page, physical_page_to_read);
    }

    // Kiểm tra physical_page_to_read một lần nữa trước khi đọc
    if (physical_page_to_read == INVALID_PAGE) {
        memset(buffer, 0xFF, PAGE_SIZE);
        return;
    }
    AT45DB_ReadPage(physical_page_to_read, buffer, NULL);
}

void FTL_Write( uint16_t logical_page, const uint8_t* buffer ) {
    /* Kiểm tra tham số đầu vào */
    if(logical_page >= TOTAL_LOGICAL_PAGES || buffer == NULL)
        return;

    // Thay đổi mới
    _Invalidate_old_physical_page(logical_page); // Đánh dấu là page invalid


    /* 1. Tìm block và page free => physical page ? */
    int16_t block_id = ftl.curr_write.block_idx;
    int8_t  page_id  = _Find_free_page(block_id, &ftl.pvb);

    if(page_id < 0) { // Block hiện tại đã hết page free
        block_id = _Find_free_block(&ftl, ftl.BlockAllocationTable);

        if (block_id < 0) {
            /* Flash full → sau này gọi GC */
            return;
        }
        page_id = _Find_free_page(block_id, &ftl.pvb);
    }
    uint16_t physical_page = BLOCK_TO_PAGE(block_id) + page_id;

    /* 2. Write data */
    spare_t spare;  // spare của block hiện tại
    spare.fields.logical_page = logical_page;
    AT45DB_WritePage(physical_page, buffer, (uint8_t *)&spare);

#ifdef DEBUG_FTL
    l2p[logical_page] = physical_page;
#endif

    /* 3. Update and Log */
    // Update block pointer
    ftl.curr_write.block_idx = block_id;

    // Update Block Allocation Table
    ftl.BlockAllocationTable[block_id].free_pages--;

    // Put log LRU cache 
    Cache_Put(&FTL_cache, logical_page, physical_page);

    // Update mapping page 
    _Update_mapping_page(logical_page, physical_page);

    // Log PVB 
    Block_SL_t curr_block;

    curr_block.BlockID     = block_id;
    curr_block.bitMap.raw  = 0;
    SET_PAGE_STATE(curr_block.bitMap.raw, page_id, PAGE_STATE_VALID);
    curr_block.erase_flag  = false;

    Put_PVB(&ftl.pvb, curr_block);

    // Gọi GC
    GC_UpdateBlock(block_id, &ftl);
    GC_OnWrite(ftl.gc);
    GC_Run(ftl.gc, &ftl);
    
}

void cachePrint(void) 
{
    Cache_Print(&FTL_cache);
}

void FTL_PrintGC(void) {
    GC_PrintBlocks();
}