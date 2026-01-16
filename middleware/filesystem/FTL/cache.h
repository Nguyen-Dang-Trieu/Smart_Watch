#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>

/* Cache operations interface */
typedef struct {
  int  (*get)  (void *ctx, int key, int *value);
  void (*put)  (void *ctx, int key, int value);
  void (*print)(void *ctx);
} Cache_ops_t;

/* Cache object */
typedef struct {
  Cache_ops_t *ops;   // các phương thức
  void        *ctx;   // context cụ thể (LRU, FIFO, ...)
} Cache_t;

/* Generic cache APIs */
void Cache_init (Cache_t *c, void *ctx, Cache_ops_t *ops);
void Cache_Put  (Cache_t *c, int key, int value);
int  Cache_Get  (Cache_t *c, int key, int *value);
void Cache_Print(Cache_t *c);

#endif /* CACHE_H */
