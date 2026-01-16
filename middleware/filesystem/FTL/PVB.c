#include "PVB.h"

/* --- Private Function Prototypes (Internal Use Only) ---------------------------------------------- */

// Selection Sort 
static void _swap( Block_SL_t *a, Block_SL_t *b );
static void _selectionSort(Block_SL_t arr[], size_t n);

/**
 * @brief Thêm or update log block 
 * @param pvb Trỏ đến tối tượng quản lí
 * @param block block vừa mới dùng
 * @retval 
 */
static int _insert_or_update( PVB_t *pvb, Block_SL_t block ); 

/**
 * @brief Đẩy dữ liệu từ memtable xuống L0
 * @param lsm Trỏ đến lsm
 */
static void _flush_L0( PVB_t *pvb );

/**
 * @brief Nhập 2 mảng lại
 * @retval Số lượng phần tử trong Temp
 */
static size_t _merge(Block_SL_t *Temp,
           Block_SL_t *array0, size_t array0_size, 
           Block_SL_t *array1, size_t array1_size); 



/* --- Define Private Functions -------------- */

static void _swap( Block_SL_t *a, Block_SL_t *b )
{
  Block_SL_t temp = *a;
  *a = *b;
  *b = temp;
}

static void _selectionSort(Block_SL_t *working_area, size_t n) 
{
    for (size_t i = 0; i < n - 1; i++) {
        size_t min_idx = i;
        for (size_t j = i + 1; j < n; j++) {
            if (working_area[j].BlockID < working_area[min_idx].BlockID) {
                min_idx = j;
            }
        }
        // Hoán đổi nếu tìm thấy phần tử nhỏ hơn
        if (min_idx != i) {
            _swap(&working_area[min_idx], &working_area[i]);
        }
    }
}

static int _insert_or_update( PVB_t *pvb, Block_SL_t block ) 
{
  // Tìm xem id đã tồn tại trong memtable chưa?
  for (uint8_t i = 0; i < pvb->memtable_size; i++) {
    if (pvb->memtable[i].BlockID == block.BlockID) { // Update nếu có 
        
        if(block.bitMap.raw == 0 && block.erase_flag == true) { // Vừa mới được GC dọn rác
          pvb->memtable[i].bitMap.raw  = 0;
          pvb->memtable[i].erase_flag  = true;
        } else {
          pvb->memtable[i].bitMap.raw  = pvb->memtable[i].bitMap.raw | block.bitMap.raw;
          pvb->memtable[i].erase_flag  = block.erase_flag; // erase_flag = false
        }
        return 0;  // Update info block
    }
  }
    
  // Nếu log của block này không có => Chưa có trên memtable, trong L0 có thể có nhưng không liên quan
  if (pvb->memtable_size < MEMTABLE_MAX_SIZE) {
      pvb->memtable[pvb->memtable_size] = block;
      pvb->memtable_size++;
      return 1;  // Add block
  }
  return -1; // Full used_blocks
}

static void _flush_L0( PVB_t *pvb )
{  
  if (!pvb || pvb->memtable_size == 0) return;

  // Sắp xếp "memtable" theo thứ tự ID trước khi flush
  _selectionSort(pvb->memtable, pvb->memtable_size);

  /* Trường hợp L0 trống: Copy trực tiếp từ memtable -> L0 */
  if (pvb->L0_size == 0) {
    memcpy(pvb->L0, pvb->memtable, pvb->memtable_size * sizeof(Block_SL_t));
    pvb->L0_size = pvb->memtable_size;
    return;
  }

  /* Trường hợp L0 đã có dữ liệu trước đó: merge */
  static Block_SL_t l0_compacted[L0_MAX_SIZE]; // Use static để tránh tràn stack
         size_t     l0_compacted_size;

  l0_compacted_size = _merge(
      l0_compacted,
      pvb->L0, pvb->L0_size,
      pvb->memtable, pvb->memtable_size
  );

  /* Ghi lại L0 sau khi đã merge và xóa trùng lặp */
  memcpy(pvb->L0, l0_compacted, l0_compacted_size * sizeof(Block_SL_t));
  pvb->L0_size = l0_compacted_size;
}

static size_t _merge(Block_SL_t *Temp,
           Block_SL_t *array0, size_t array0_size, 
           Block_SL_t *array1, size_t array1_size) 
{
  uint8_t index_temp = 0, index_array0 = 0, index_array1 = 0;
  while(index_array0 < array0_size && index_array1 < array1_size) {
    if(array0[index_array0].BlockID == array1[index_array1].BlockID) {
      Block_SL_t Temp_entry;

      Temp_entry.BlockID = array0[index_array0].BlockID;
      
      if(array0[index_array0].erase_flag == false && array1[index_array1].erase_flag == true) {
        Temp_entry.bitMap.raw  = 0; // Reset toàn bộ bitMap do block mới được GC
        Temp_entry.erase_flag  = true;
      } else {
        Temp_entry.bitMap.raw  = array0[index_array0].bitMap.raw | array1[index_array1].bitMap.raw;
        Temp_entry.erase_flag  = false;
      }
       
      index_array0++, index_array1++;
      Temp[index_temp++] = Temp_entry;

    } else if(array0[index_array0].BlockID < array1[index_array1].BlockID) {
      Temp[index_temp++] = array0[index_array0++];
    } else {
      Temp[index_temp++] = array1[index_array1++];
    }
  }

  // Xử lí khi một trong hai mảng kết thúc trước, còn lại một số phần tử chưa ghi vào Temp
  while(index_array0 < array0_size) Temp[index_temp++] = array0[index_array0++];
  while(index_array1 < array1_size) Temp[index_temp++] = array1[index_array1++];

  return index_temp; // Số lượng phần tử trong Temp
}



/* --- External Function Definitions (Public Interface) ---------------------------------------------- */

void Put_PVB( PVB_t *pvb, Block_SL_t block )
{
    if (!pvb) return;

    // Ghi log của block vào memtable
    _insert_or_update(pvb, block);

    // Nếu memtable đầy, flush xuống L0
    if (pvb->memtable_size >= MEMTABLE_MAX_SIZE) {
      _flush_L0(pvb);
      pvb->memtable_size = 0;
    }
}

Block_SL_t Get_PVB( PVB_t *pvb, uint16_t id ) 
{
  Block_SL_t block_to_search = {0};
  Block_SL_t *search_area = pvb->memtable;

  // Tìm trong memtable trước
  for(uint8_t i = 0; i < pvb->memtable_size; i++) {

    if(search_area[i].BlockID == id) {

      // Log mới nhất của block là vừa được GC dọn rác
      if(search_area[i].erase_flag == true) {
        return block_to_search = search_area[i];

      } else if(search_area[i].erase_flag == false) {
        block_to_search.BlockID    = search_area[i].BlockID;
        block_to_search.bitMap     = search_area[i].bitMap;
        block_to_search.erase_flag = search_area[i].erase_flag;
      }
    }
  }

  // Tìm trong L0 -> Cải tiến thuật toán dùng Binarry sort vì L0 đã được sắp xếp
  search_area = pvb->L0;
  for(uint8_t i = 0; i < pvb->L0_size; i++) {

    if(search_area[i].BlockID == id) {

      if(search_area[i].erase_flag == true) {
        return block_to_search;
      } else if(search_area[i].erase_flag == false) {
        block_to_search.bitMap.raw = block_to_search.bitMap.raw | search_area[i].bitMap.raw;
      }
    }
  }

  return block_to_search;
}

void printMemtable( PVB_t *pvb )
{
  printf("\n* Memtable (size = %zu):\n", pvb->memtable_size);
  for (size_t i = 0; i < pvb->memtable_size; i++)
  {
    printf("Block ID = %u, bitMap = 0x%04X, erase_flag = %d\n",
       (unsigned)pvb->memtable[i].BlockID,
       pvb->memtable[i].bitMap.raw,
       pvb->memtable[i].erase_flag ? 1 : 0);
  }
}

void printL0( PVB_t *pvb )
{
  printf("\n* L0\n");
  for (size_t i = 0; i < pvb->L0_size; i++)
  {
    printf("  Block ID = %d, bitMap = 0x%04X, erase_flag = %d\n",
           pvb->L0[i].BlockID,
           pvb->L0[i].bitMap.raw,
           pvb->L0[i].erase_flag);
  }
}

