#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define TOTAL_BLOCKS          10
#define GET_PARENT(i)         ((i - 1) / 2)
#define GET_LEFT_CHILD(i)     ((2 * i) + 1)
#define GET_RIGHT_CHILD(i)    ((2 * i) + 2)
#define NOT_IN_HEAP           0xFFFE
#define WEAR_SHIFT            5 // Viết document để giải thích 


// Định nghĩa struct trước
typedef struct {
    uint16_t block_id;
    uint16_t cost;
} HeapNode_t;

// Biến toàn cục
HeapNode_t heap[TOTAL_BLOCKS];
uint16_t pos_in_heap[TOTAL_BLOCKS]; 
uint16_t size = 0;

void Init_pos(void) 
{
  for(uint16_t i = 0; i < TOTAL_BLOCKS; i++) {
    pos_in_heap[i] = NOT_IN_HEAP;
  }
}

uint16_t Calculate_cost(uint16_t V, uint16_t E) 
{
  // V là số valid, E là số lần xóa của block -> wear leveling
  return V + (E >> WEAR_SHIFT); 
}

void swap(int a, int b) {
    // Cập nhật bản đồ vị trí dựa trên block_id
    pos_in_heap[heap[a].block_id] = b;
    pos_in_heap[heap[b].block_id] = a;
    
    // Đổi chỗ dữ liệu
    HeapNode_t temp = heap[a];
    heap[a] = heap[b];
    heap[b] = temp;
}

void HeapifyUp(int idx) {
    while (idx > 0) {
        int parent = GET_PARENT(idx);
        if (heap[idx].cost < heap[parent].cost) {
            swap(idx, parent);
            idx = parent;
        } else break;
    }
}

void HeapifyDown(int idx) {
    while (true) {
        int smallest = idx;
        int left = GET_LEFT_CHILD(idx);
        int right = GET_RIGHT_CHILD(idx);
        
        if (left < size && heap[left].cost < heap[smallest].cost) smallest = left;
        if (right < size && heap[right].cost < heap[smallest].cost) smallest = right;
        
        if (smallest != idx) {
            swap(idx, smallest);
            idx = smallest;
        } else break;
    }
}

// Thêm block mới vào hệ thống (thường gọi lúc khởi tạo FTL)
void Insert_Block(uint16_t block_id, uint16_t V, uint16_t E) {
    if (size >= TOTAL_BLOCKS) return;
    
    int idx = size;
    
    heap[idx].block_id = block_id;
    heap[idx].cost     = Calculate_cost(V, E);
    
    pos_in_heap[block_id] = idx;
    
    size++;
    HeapifyUp(idx);
}

void Update_Block(uint16_t block_id, uint16_t New_V, uint16_t New_E) {
    int idx = pos_in_heap[block_id];
    uint16_t old_cost = heap[idx].cost;
    uint16_t new_cost = Calculate_cost(New_V, New_E);
    
    heap[idx].cost = new_cost;
    
    if (new_cost < old_cost) HeapifyUp(idx);
    else HeapifyDown(idx);
}



void Mark_Block_Activity(uint16_t block_id, uint16_t V, uint16_t E) {
    if(pos_in_heap[block_id] == NOT_IN_HEAP) {
        Insert_Block(block_id, V, E);
    } else {
        Update_Block(block_id, V, E);
    }
}

void display() {
    printf("Heap (ID:Cost): ");
    for (int i = 0; i < size; i++) {
        printf("[%d:%d] ", heap[i].block_id, heap[i].cost);
    }
    printf("\n");
}

int main() {
    Init_pos();
  
    // Sử dụng Mark_Block_Activity thay vì gọi trực tiếp Insert/Update
    printf("--- Khoi tao cac block ---\n");
    Mark_Block_Activity(0, 100, 10); 
    Mark_Block_Activity(1, 50,  20); 
    Mark_Block_Activity(2, 10,  30); 
    Mark_Block_Activity(3, 80,  5);
    
    display(); 
    printf("Victim hien tai (Cost thap nhat): Block %d\n\n", heap[0].block_id);

    printf("--- Cap nhat Block 1 (Invalidate du lieu) ---\n");
    // Giả sử dữ liệu cũ bị xóa nhiều, Valid Pages giảm mạnh
    Mark_Block_Activity(1, 5, 20); 
    
    display();
    printf("Victim moi: Block %d\n", heap[0].block_id);

    return 0;
}