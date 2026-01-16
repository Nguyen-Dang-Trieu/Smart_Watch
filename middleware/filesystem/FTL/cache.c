#include "cache.h"

void Cache_init(Cache_t *c, void *ctx, Cache_ops_t *ops)
{
  c->ops = ops;
  c->ctx = ctx;
}

void Cache_Put(Cache_t *c, int key, int value)
{
  c->ops->put(c->ctx, key, value);
}

int Cache_Get(Cache_t *c, int key, int *value)
{
  return c->ops->get(c->ctx, key, value);
}

void Cache_Print(Cache_t *c)
{
  c->ops->print(c->ctx);
}
