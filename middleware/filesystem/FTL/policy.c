#include "policy.h"

// Static global variable
static size_t     curren_heap_size = 0;   // Số lượng phần tử hiện tại có trong Heap
static HeapNode_t Heap[MAX_HEAP];         // Min Heap
static uint16_t   pos_in_heap[MAX_HEAP];  // Đánh dấu vị trí của các block nằm trong Heap

/* --- Private Function Prototypes (Internal Use Only) ---------------------------------------------- */

static void Init_position( void ) 
{
  for(uint16_t i = 0; i < MAX_HEAP; i++) {
    pos_in_heap[i] = NOT_IN_HEAP;
  }
}

static uint16_t Calculate_cost( uint16_t V, uint16_t E ) 
{
    return V + (E >> WEAR_SHIFT); 
}

static void swap(int a, int b) {
    // Cập nhật bản đồ vị trí dựa trên block_id
    pos_in_heap[Heap[a].block_id] = b;
    pos_in_heap[Heap[b].block_id] = a;
    
    // Đổi chỗ dữ liệu
    HeapNode_t temp = Heap[a];
    Heap[a] = Heap[b];
    Heap[b] = temp;
}

static void HeapifyUp(int idx) {
    while (idx > 0) {
        int parent = GET_PARENT(idx);
        if (Heap[idx].cost < Heap[parent].cost) {
            swap(idx, parent);
            idx = parent;
        } else break;
    }
}

static void HeapifyDown(size_t idx) {
    while (true) {
        size_t smallest = idx;
        size_t left = GET_LEFT_CHILD(idx);
        size_t right = GET_RIGHT_CHILD(idx);
        
        if (left < curren_heap_size && Heap[left].cost < Heap[smallest].cost) smallest = left;
        if (right < curren_heap_size && Heap[right].cost < Heap[smallest].cost) smallest = right;
        
        if (smallest != idx) {
            swap(idx, smallest);
            idx = smallest;
        } else break;
    }
}

static void Insert_Block(uint16_t block_id, uint16_t V, uint16_t E) {
    if (curren_heap_size >= MAX_HEAP) return;
    
    int idx = curren_heap_size;
    
    Heap[idx].block_id = block_id;
    Heap[idx].cost     = Calculate_cost(V, E);
    
    pos_in_heap[block_id] = idx;
    
    curren_heap_size++;
    HeapifyUp(idx);
}

static void Update_Block(uint16_t block_id, uint16_t New_V, uint16_t New_E) {
    int idx = pos_in_heap[block_id];
    uint16_t old_cost = Heap[idx].cost;
    uint16_t new_cost = Calculate_cost(New_V, New_E);
    
    Heap[idx].cost = new_cost;
    
    if (new_cost < old_cost) HeapifyUp(idx);
    else HeapifyDown(idx);
}

static void Mark_Block_Activity(uint16_t block_id, uint16_t V, uint16_t E) {
    if(pos_in_heap[block_id] == NOT_IN_HEAP) {
        Insert_Block(block_id, V, E);
    } else {
        Update_Block(block_id, V, E);
    }
}

/* --- External Function Definitions (Public Interface) ---------------------------------------------- */
void Policy_Init( void ) 
{
    curren_heap_size = 0;
    Init_position();
}

void Policy_UpdateOrInsert( uint16_t block_id, uint16_t valid_pages, uint16_t erase_count )
{
    Mark_Block_Activity(block_id, valid_pages, erase_count);
}

uint16_t Policy_GetVictim( void )
{
    if(curren_heap_size == 0) {
        return EMPTY_HEAP;
    }    
    return Heap[0].block_id;
}

void Policy_OnErase(uint16_t block_id, uint16_t new_erase_count) 
{
    Mark_Block_Activity(block_id, 0, new_erase_count);
}

void Policy_PrintHeap(void)
{
    printf("\n=========== GC POLICY HEAP ===========\n");
    printf("Heap size : %lld\n", curren_heap_size);

    if (curren_heap_size == 0) {
        printf("Heap is EMPTY\n");
        printf("=====================================\n");
        return;
    }

    printf("Idx | Block | Cost | pos_in_heap\n");
    printf("-------------------------------------\n");

    for (uint16_t i = 0; i < curren_heap_size; i++) {
        uint16_t blk = Heap[i].block_id;
        printf("%3d | %5d | %4d | %5d",
               i,
               blk,
               Heap[i].cost,
               pos_in_heap[blk]);

        if (i == 0)
            printf("  <== VICTIM");

        printf("\n");
    }

    printf("-------------------------------------\n");
    printf("Current victim block: %d\n", Heap[0].block_id);
    printf("=====================================\n");
}
