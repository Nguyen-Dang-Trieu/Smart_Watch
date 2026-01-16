#ifndef LRU_H
#define LRU_H

#include <stdbool.h>
#include <stdint.h>

#define LRU_CACHE_SIZE 5 

typedef struct LRU_Node {
  int key;
  int value;
  struct LRU_Node *prev, *next;
} LRU_Node_t;

typedef struct {
  LRU_Node_t *head;
  LRU_Node_t *tail;
  int         size;
} LRU_List_t;


typedef struct LRU {
  LRU_List_t list;
  bool       used_pool[LRU_CACHE_SIZE];
  void      *node_pool;
} LRU_t;

/* LRU public APIs */
void LRU_Init (LRU_t *lru);
int  LRU_Get  (LRU_t *lru, int key);
void LRU_Put  (LRU_t *lru, int key, int value);
void LRU_Print(LRU_t *lru);

/* Adapter ops for generic cache */
#include "cache.h"
extern Cache_ops_t LRU_ops;

#endif /* LRU_H */
