#include "siera/ds_null.h"

static void null_read(i_siera_ds_t* interface, siera_dsk_t key, void* buf)
{
  (void)interface;
  (void)key;
  (void)buf;
}

static void null_write(i_siera_ds_t* interface, siera_dsk_t key, const void* buf)
{
  (void)interface;
  (void)key;
  (void)buf;
}

static bool null_contains(i_siera_ds_t* interface, siera_dsk_t key)
{
  (void)interface;
  (void)key;
  return false;
}

static size_t null_size(i_siera_ds_t* interface, siera_dsk_t key)
{
  (void)interface;
  (void)key;
  return 0;
}

static siera_event_t* null_on_change(i_siera_ds_t* interface)
{
  siera_ds_null_t* instance = (siera_ds_null_t*)interface;
  return &instance->on_change;
}

static const i_siera_ds_api_t _null_api = {
  .read = null_read,
  .write = null_write,
  .contains = null_contains,
  .size = null_size,
  .on_change = null_on_change,
};

void siera_ds_null_init(siera_ds_null_t* instance)
{
  instance->interface.api = &_null_api;
  siera_event_init(&instance->on_change);
}
