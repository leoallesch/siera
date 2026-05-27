#ifndef DS_H
#define DS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "siera/dsk.h"
#include "siera/event.h"

typedef struct {
  siera_dsk_t key;
  const void* data;
} siera_ds_on_change_args_t;

struct i_siera_ds_api_t;

typedef struct {
  const struct i_siera_ds_api_t* api;
} i_siera_ds_t;

typedef struct i_siera_ds_api_t {
  void (*read)(i_siera_ds_t* inteface, siera_dsk_t key, void* buf);
  void (*write)(i_siera_ds_t* inteface, siera_dsk_t key, const void* buf);
  bool (*contains)(i_siera_ds_t* inteface, siera_dsk_t key);
  size_t (*size)(i_siera_ds_t* inteface, siera_dsk_t key);
  siera_event_t* (*on_change)(i_siera_ds_t* inteface);
} i_siera_ds_api_t;

static inline void siera_ds_read(i_siera_ds_t* inteface, siera_dsk_t key, void* buf)
{
  return inteface->api->read(inteface, key, buf);
}

static inline void siera_ds_write(i_siera_ds_t* inteface, siera_dsk_t key, const void* buf)
{
  return inteface->api->write(inteface, key, buf);
}

static inline bool siera_ds_contains(i_siera_ds_t* inteface, siera_dsk_t key)
{
  return inteface->api->contains(inteface, key);
}

static inline size_t siera_ds_size(i_siera_ds_t* inteface, siera_dsk_t key)
{
  return inteface->api->size(inteface, key);
}

static inline siera_event_t* siera_ds_on_change(i_siera_ds_t* inteface)
{
  return inteface->api->on_change(inteface);
}

static inline void siera_ds_subscribe_all(i_siera_ds_t* inteface, siera_event_sub_t* sub)
{
  siera_event_subscribe(siera_ds_on_change(inteface), sub);
}

static inline void siera_ds_unsubscribe_all(i_siera_ds_t* inteface, siera_event_sub_t* sub)
{
  siera_event_unsubscribe(siera_ds_on_change(inteface), sub);
}

#endif
