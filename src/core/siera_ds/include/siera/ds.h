#ifndef SIERA_DS_H
#define SIERA_DS_H

#include <stddef.h>
#include <stdint.h>
#include "siera/ds_keys.h"
#include "siera/ds_stream.h"
#include "siera/event.h"
#include "siera/timer.h"

typedef struct {
  siera_ds_key_t key;
  const void* data;
} siera_ds_on_change_t;

typedef struct {
  siera_ds_stream_type_t type;
  siera_ds_stream_t* stream;
} siera_ds_stream_binding_t;

/*
 * ds_keys_internal.h generates siera_ds_cache_t, siera_ds_entry_t, and
 * _siera_ds_entry_table from the user-supplied siera_ds_keys.def.
 *
 * Consumers must ensure that the directory containing siera_ds_keys.def is
 * on their include path (see CMakeLists for siera_ds — it exposes the def
 * directory as PUBLIC so linking against siera_ds is sufficient).
 */
#include "ds_keys_internal.h"

typedef struct {
  const siera_ds_entry_t* entries;
  siera_ds_cache_t cache;
  siera_event_t events;
  siera_timer_mgr_t* timers;
  uint32_t flush_interval_ms;
  uint32_t dirty[(SIERA_DS_KEY_COUNT + 31) / 32];
  siera_ds_stream_t* stream_map[SIERA_DS_STREAM_TYPE_COUNT];
  siera_timer_t flush_timer;
} siera_ds_t;

int siera_ds_init(siera_ds_t* ds,
  const siera_ds_stream_binding_t* streams,
  size_t stream_count,
  siera_timer_mgr_t* timers,
  uint32_t flush_interval_ms);
int siera_ds_read(const siera_ds_t* ds, siera_ds_key_t key, void* out);
int siera_ds_write(siera_ds_t* ds, siera_ds_key_t key, const void* in);
void siera_ds_deinit(siera_ds_t* ds);

int siera_ds_subscribe_all(siera_ds_t* ds, siera_event_sub_t* sub);
int siera_ds_unsubscribe_all(siera_ds_t* ds, siera_event_sub_t* sub);

const char* siera_ds_key_name(siera_ds_key_t key);
size_t siera_ds_key_size(siera_ds_key_t key);
siera_ds_stream_type_t siera_ds_key_stream_type(siera_ds_key_t key);

#endif
