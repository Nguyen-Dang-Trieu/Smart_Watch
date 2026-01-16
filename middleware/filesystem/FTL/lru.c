#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lru.h"


/* ================= Memory pool helpers ================= */

static void *allocate_node_From_Pool(LRU_t *lru)
{
  uint8_t *base = (uint8_t *)lru->node_pool;

  for (uint8_t i = 0; i < LRU_CACHE_SIZE; i++) {
    if (!lru->used_pool[i]) {
      lru->used_pool[i] = true;
      return base + i * sizeof(LRU_Node_t);
    }
  }
  return NULL;
}

static void deallocate_node_To_Pool(LRU_t *lru, void *node_ptr)
{
  if (!node_ptr || !lru->node_pool) return;

  size_t offset = (uint8_t *)node_ptr - (uint8_t *)lru->node_pool;
  uint8_t index = offset / sizeof(LRU_Node_t); 

  if (index < LRU_CACHE_SIZE)
    lru->used_pool[index] = false;
}

static LRU_Node_t *CreateNode(LRU_t *lru, int key, int value)
{
  LRU_Node_t *node = allocate_node_From_Pool(lru);
  if (!node) return NULL;

  node->key = key;
  node->value = value;
  node->prev = node->next = NULL;
  return node;
}

/* ================= LRU list helpers ================= */

static void moveToHead(LRU_Node_t *node, LRU_List_t *list)
{
  if (!node || node == list->head) return;

  if (node->prev) node->prev->next = node->next;
  if (node->next) node->next->prev = node->prev;
  if (node == list->tail) list->tail = node->prev;

  node->prev = NULL;
  node->next = list->head;
  if (list->head) list->head->prev = node;
  list->head = node;
}

/* ================= LRU APIs ================= */

void LRU_Init(LRU_t *lru)
{
  size_t size = sizeof(LRU_Node_t) * LRU_CACHE_SIZE;
  lru->node_pool = malloc(size);

  if (!lru->node_pool) return;

  memset(lru->node_pool, 0, size);
  memset(lru->used_pool, 0, sizeof(lru->used_pool));
  memset(&lru->list, 0, sizeof(lru->list));
}

int LRU_Get(LRU_t *lru, int key)
{
  LRU_Node_t *cur = lru->list.head;

  while (cur) {
    if (cur->key == key) {
      moveToHead(cur, &lru->list);
      return cur->value;
    }
    cur = cur->next;
  }
  return -1;
}

void LRU_Put(LRU_t *lru, int key, int value)
{
  LRU_List_t *list = &lru->list;

  /* Update existing */
  for (LRU_Node_t *n = list->head; n; n = n->next) {
    if (n->key == key) {
      n->value = value;
      moveToHead(n, list);
      return;
    }
  }

  /*  If the place is full, they have to kick last node out. */
  if (list->size >= LRU_CACHE_SIZE) {
    LRU_Node_t *last_node = list->tail;

    // Xóa khỏi list, ngắt kết nối last node với node trước 
    if(last_node->prev) {
      last_node->prev->next = NULL;
    } else
      list->head = NULL;
    list->tail = last_node->prev;

    // last node
    last_node->next = last_node->prev = NULL;

    deallocate_node_To_Pool(lru, last_node);
    list->size--;
  }

  /* Insert new */
  LRU_Node_t *node = CreateNode(lru, key, value);
  if (!node) return;

  node->next = list->head;
  if (list->head) list->head->prev = node;
  list->head = node;

  if (!list->tail) list->tail = node;
  list->size++;
}

void LRU_Print(LRU_t *lru)
{
  printf("Cache size: %d\n", lru->list.size);
  for (LRU_Node_t *n = lru->list.head; n; n = n->next) {
    printf("[%d:%d]", n->key, n->value);
    if (n->next) printf(" <-> ");
  }
  printf("\n");
}


/**
 * @brief  Hàm adapter để truy vấn giá trị từ cấu trúc dữ liệu LRU.
 * * Hàm này đóng vai trò là một wrapper cho hàm LRU_Get, giúp tương thích
 * * với các interface yêu cầu giá trị trả về là mã lỗi và nhận kết quả qua con trỏ.
 * @param  ctx   Con trỏ vô kiểu (void *), thực chất là con trỏ đến cấu trúc LRU (LRU_t *).
 * @param  key   Key cần tìm kiếm trong bộ đệm LRU.
 * @param  value Con trỏ dùng để lưu trữ giá trị tìm thấy nếu key tồn tại.
 * @return int   Kết quả thực thi:
 * 
 * 1: Tìm thấy khóa thành công, giá trị được lưu vào *value.
 * 
 * -1: Không tìm thấy khóa hoặc có lỗi xảy ra (LRU_Get trả về < 0).
 */

static int LRU_Get_adapter(void *ctx, int key, int *value)
{
  int v = LRU_Get((LRU_t *)ctx, key);
  if (v < 0) return -1;
  *value = v;
  return 1;
}


static void LRU_Put_adapter(void *ctx, int key, int value)
{
  LRU_Put((LRU_t *)ctx, key, value);
}

static void LRU_Print_adapter(void *ctx)
{
  LRU_Print((LRU_t *)ctx);
}

Cache_ops_t LRU_ops = {
  .get   = LRU_Get_adapter,
  .put   = LRU_Put_adapter,
  .print = LRU_Print_adapter
};
