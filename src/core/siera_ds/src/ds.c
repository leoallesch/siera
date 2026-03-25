/*
 * This file is compiled into the siera library.  It requires that the
 * directory containing siera_ds_conf.h is on the include path — either
 * the user's own config or the library's built-in default will be picked
 * up automatically via the __has_include() check in ds_keys.h.
 */
#include <string.h>
#include <stdbool.h>
#include "siera/ds.h"
#include "ds_keys_internal.h"

static siera_ds_cache_t _ds_cache;
static bool _ds_dirty[DSK_COUNT];

/* ── Entry table ────────────────────────────────────────────────────────── */

#define _DS_EXPAND_ENTRY(name, type, st, dv, fl) \
  { #name, (st), sizeof(type), SIERA_OFFSET_OF(siera_ds_cache_t, name), (fl) },

const siera_ds_entry_t _siera_ds_entry_table[] = {
  SIERA_DS_KEYS(_DS_EXPAND_ENTRY)
};

#undef _DS_EXPAND_ENTRY

static inline const siera_ds_entry_t* ds_find_entry(siera_dsk_t key)
{
  return (key < DSK_COUNT) ? &_siera_ds_entry_table[key] : NULL;
}

static inline siera_ds_stream_t* ds_stream(const siera_ds_t* ds,
  siera_ds_stream_type_t type)
{
  return (type < SIERA_DS_STREAM_TYPE_COUNT) ? ds->stream_map[type] : NULL;
}

static void ds_flush(void* ctx)
{
  siera_ds_t* ds = (siera_ds_t*)ctx;
  siera_ds_stream_t* s = ds_stream(ds, SIERA_DS_NVS);
  if(!s || !s->api->write)
    return;
  for(int i = 0; i < DSK_COUNT; i++) {
    if(_ds_dirty[i]) {
      const siera_ds_entry_t* e = &ds->entries[i];
      if(s->api->write(s->ctx, (siera_dsk_t)i, (const uint8_t*)&_ds_cache + e->offset, e->size) == 0)
        _ds_dirty[i] = false;
    }
  }
}

int siera_ds_init(siera_ds_t* ds,
  const siera_ds_stream_binding_t* streams,
  size_t stream_count,
  siera_timer_mgr_t* timers,
  uint32_t flush_interval_ms)
{
  memset(ds, 0, sizeof(*ds));
  ds->entries = _siera_ds_entry_table;
  siera_event_init(&ds->events);
  ds->timers = timers;
  ds->flush_interval_ms = flush_interval_ms;

  for(size_t i = 0; i < stream_count; i++) {
    if(streams[i].type < SIERA_DS_STREAM_TYPE_COUNT)
      ds->stream_map[streams[i].type] = streams[i].stream;
  }

  _siera_ds_apply_defaults(&_ds_cache);

  /* Hydrate persist and hardware keys from their streams */
  for(int i = 0; i < DSK_COUNT; i++) {
    const siera_ds_entry_t* e = &ds->entries[i];
    if(e->stream_type == SIERA_DS_RAM)
      continue;
    siera_ds_stream_t* s = ds_stream(ds, e->stream_type);
    if(s && s->api->read)
      s->api->read(s->ctx, (siera_dsk_t)i, (uint8_t*)&_ds_cache + e->offset, e->size);
  }

  /* Start auto-flush timer if interval > 0 */
  if(ds->flush_interval_ms > 0 && ds->timers) {
    siera_timer_start(ds->timers, &ds->flush_timer, ds_flush, ds, ds->flush_interval_ms, true);
  }

  return 0;
}

int siera_ds_read(const siera_ds_t* ds, siera_dsk_t key, void* out)
{
  const siera_ds_entry_t* e = ds_find_entry(key);
  if(!e)
    return -1;

  /* For hardware-backed keys, sample fresh from stream */
  if(e->stream_type >= SIERA_DS_HARDWARE_START) {
    siera_ds_stream_t* s = ds_stream(ds, e->stream_type);
    if(s && s->api->read)
      s->api->read(s->ctx, key, (uint8_t*)&_ds_cache + e->offset, e->size);
  }

  memcpy(out, (const uint8_t*)&_ds_cache + e->offset, e->size);
  return 0;
}

int siera_ds_write(siera_ds_t* ds, siera_dsk_t key, const void* in)
{
  const siera_ds_entry_t* e = ds_find_entry(key);
  if(!e)
    return -1;
  if(e->flags & SIERA_DS_READONLY)
    return -1;

  uint8_t* cached = (uint8_t*)&_ds_cache + e->offset;
  if(memcmp(cached, in, e->size) == 0)
    return 0; /* no change */

  memcpy(cached, in, e->size);

  if(e->stream_type == SIERA_DS_NVS && ds->flush_interval_ms > 0) {
    _ds_dirty[key] = true;
  }
  else if(e->stream_type != SIERA_DS_RAM) {
    siera_ds_stream_t* s = ds_stream(ds, e->stream_type);
    if(s && s->api->write)
      s->api->write(s->ctx, key, in, e->size);
  }

  /* Publish change event */
  if(!(e->flags & SIERA_DS_SILENT)) {
    siera_ds_on_change_t change = { .key = key, .data = in };
    siera_event_publish(&ds->events, &change);
  }
  return 0;
}

int siera_ds_subscribe_all(siera_ds_t* ds, siera_event_sub_t* sub)
{
  if(!ds)
    return -1;
  return siera_event_subscribe(&ds->events, sub);
}

int siera_ds_unsubscribe_all(siera_ds_t* ds, siera_event_sub_t* sub)
{
  if(!ds)
    return -1;
  return siera_event_unsubscribe(&ds->events, sub);
}

void siera_ds_deinit(siera_ds_t* ds)
{
  if(ds->flush_interval_ms > 0 && ds->timers)
    siera_timer_stop(ds->timers, &ds->flush_timer);
  ds_flush(ds);
}

const char* siera_ds_key_name(siera_dsk_t key)
{
  const siera_ds_entry_t* e = ds_find_entry(key);
  return e ? e->name : "?";
}

size_t siera_ds_key_size(siera_dsk_t key)
{
  const siera_ds_entry_t* e = ds_find_entry(key);
  return e ? e->size : 0;
}

siera_ds_stream_type_t siera_ds_key_stream_type(siera_dsk_t key)
{
  const siera_ds_entry_t* e = ds_find_entry(key);
  return e ? e->stream_type : SIERA_DS_RAM;
}
