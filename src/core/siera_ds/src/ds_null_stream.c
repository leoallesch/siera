#include "siera/ds_stream.h"

static int null_read(void* ctx, siera_dsk_t key, void* buf, size_t size)
{
  (void)ctx;
  (void)key;
  (void)buf;
  (void)size;
  return 0;
}

static int null_write(void* ctx, siera_dsk_t key, const void* buf, size_t size)
{
  (void)ctx;
  (void)key;
  (void)buf;
  (void)size;
  return 0;
}

static const siera_ds_stream_api_t _null_api = { null_read, null_write };
siera_ds_stream_t siera_null_stream = { &_null_api, NULL };
