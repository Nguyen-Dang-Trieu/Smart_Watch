#include "GC.h"
#include "FTL.h"

GCContext_t GC_Module;

/* --- Private Function Prototypes (Internal Use Only) ---------------------------------------------- */
void GC_CleanVictimBlocK(uint16_t id, FTL_t *ftl);
static void GC_Finish( void ); 

/* --- Define Private Functions ---------------------------------- */

void GC_Init( GCContext_t *gc ) 
{
  // Khởi tạo gc
  gc->active        = false;
  gc->write_count   = 0;
  gc->victim_block  = 0xFFFF;

  Policy_Init();
}


void GC_OnWrite( GCContext_t * gc)
{
  gc->write_count++;
  if(gc->write_count == GC_TRIGGER) {
    gc->write_count = 0;
    gc->active      = true;
  }
}


static uint16_t GC_SelectVictim( void ) 
{
  return Policy_GetVictim();
}

// Hiện tại khi GC_Run thực thi chỉ dọn rác 1 block
void GC_Run( GCContext_t *gc, FTL_t *ftl) 
{
  if(gc->active == false) return;

  /* 1. Chọn victim block từ policy */
  uint16_t victim = GC_SelectVictim();

  if(victim == EMPTY_HEAP) {
    GC_Finish();
    return;
  }
  
  gc->victim_block = victim;

  GC_CleanVictimBlocK(gc->victim_block, ftl);
}

void GC_CleanVictimBlocK(uint16_t id, FTL_t *ftl)
{
  Block_SL_t state_of_victim = Get_PVB(&ftl->pvb, id);

  uint8_t data_buf[512]; 
  spare_t spare_buf;

  // Tiến hành xử lí từng page
  for(uint8_t p = 0; p < PAGES_PER_BLOCK; p++) {
    uint8_t page = GET_PAGE_STATE(state_of_victim.bitMap.raw, p);
    if(page == PAGE_STATE_VALID) {
      GC_ReadValidPage(id, p, data_buf, (uint8_t *)&spare_buf);
      GC_WriteNewPage(ftl, spare_buf.fields.logical_page, data_buf);
    }
  }

  // Sau khi đã di dời từng page live dữ liệu đi chỗ khác, tiến hành xóa block
  GC_EraseBlocK(ftl, id);
  GC_Finish();
}

// Tìm page còn sống và đọc, và lưu nó vào trong RAM
void GC_ReadValidPage( uint16_t block_id, uint8_t page, uint8_t *data_buf, uint8_t *spare_buf)
{
  AT45DB_ReadPage(BLOCK_TO_PAGE(block_id) + page, data_buf, spare_buf);
}

void GC_WriteNewPage( FTL_t *ftl, uint16_t lp,  uint8_t *data_buf)
{
  // 1. Tìm block + page free trong curr_write
    int16_t block_id = ftl->curr_write.block_idx;
    int8_t  page_id  = _Find_free_page(block_id, &ftl->pvb);

    if(page_id < 0) {
        block_id = _Find_free_block(ftl, ftl->BlockAllocationTable);
        page_id = _Find_free_page(block_id, &ftl->pvb);
    }
    uint16_t pp = BLOCK_TO_PAGE(block_id) + page_id;

    // 2. Ghi dữ liệu trực tiếp, KHÔNG invalidate old page
    spare_t spare;
    spare.fields.logical_page = lp;
    AT45DB_WritePage(pp, data_buf, (uint8_t *)&spare);

    // 3. Cập nhật PVB
    Block_SL_t log_Of_currBlock;
    log_Of_currBlock.BlockID    = block_id;
    log_Of_currBlock.bitMap.raw = 0;
    SET_PAGE_STATE(log_Of_currBlock.bitMap.raw, page_id, PAGE_STATE_VALID);
    log_Of_currBlock.erase_flag = false;
    Put_PVB(&ftl->pvb, log_Of_currBlock);

    // 4. Cập nhật mapping page và cache
    _Update_mapping_page(lp, pp);
    Cache_Put(&FTL_cache, lp, pp);

    // 6. Update curr_write block pointer và free_pages
    ftl->curr_write.block_idx = block_id;
    ftl->curr_write.page_idx  = page_id + 1;;
    ftl->BlockAllocationTable[block_id].free_pages--;
}

void GC_EraseBlocK( FTL_t* ftl, uint16_t id_of_victimblock )
{
  // Xóa dưới flash
  AT45DB_EraseBlock(id_of_victimblock);

  // Cập nhập lại trạng thái của block vừa mới được xóa trên RAM
  ftl->BlockAllocationTable[id_of_victimblock].free_pages    = PAGES_PER_BLOCK;
  ftl->BlockAllocationTable[id_of_victimblock].invalid_pages = 0;
  ftl->BlockAllocationTable[id_of_victimblock].erase_count++;

  // Ghi log trạng thái mới nhất của victim block cho PVB 
  Block_SL_t clean_block = { .BlockID = id_of_victimblock, .bitMap.raw = 0, .erase_flag = true };
  Put_PVB(&ftl->pvb, clean_block);
}

static void GC_Finish( void ) 
{
  GC_Module.active       = false;
  GC_Module.victim_block = 0xFFFF; // Reset bộ đếm để bắt đầu chu kỳ mới
  GC_Module.write_count  = 0;      // Xóa victim block hiện tại
}

void GC_UpdateBlock(uint16_t block_id, FTL_t *ftl) 
{
  uint16_t valid_pages = PAGES_PER_BLOCK 
                           - ftl->BlockAllocationTable[block_id].free_pages
                           - ftl->BlockAllocationTable[block_id].invalid_pages;

  uint16_t erase_count = ftl->BlockAllocationTable[block_id].erase_count;

  Policy_UpdateOrInsert(block_id, valid_pages, erase_count);
}

void GC_PrintBlocks(void) 
{
  Policy_PrintHeap();
}
