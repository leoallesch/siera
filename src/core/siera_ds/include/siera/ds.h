#ifndef SIERA_DS_H
#define SIERA_DS_H

#include <stddef.h>
#include <stdint.h>
#include "siera/dsk.h"
#include "siera/ds_stream.h"
#include "siera/event.h"
#include "siera/timer.h"

typedef struct {
  siera_dsk_t key;
  const void* data;
} siera_ds_on_change_t;

typedef enum {
    SIERA_DS_NONE     = 0,
    SIERA_DS_READONLY = (1 << 0),
    SIERA_DS_SILENT   = (1 << 1),
} siera_ds_flags_t;

typedef struct {
    const char             *name;
    siera_ds_stream_type_t  stream_type;
    size_t                  size;
    size_t                  offset;
    siera_ds_flags_t        flags;
} siera_ds_entry_t;

extern const siera_ds_entry_t _siera_ds_entry_table[];

typedef struct {
  siera_ds_stream_type_t type;
  siera_ds_stream_t* stream;
} siera_ds_stream_binding_t;

typedef struct {
  const siera_ds_entry_t* entries;
  siera_event_t events;
  siera_timer_mgr_t* timers;
  uint32_t flush_interval_ms;
  siera_ds_stream_t* stream_map[SIERA_DS_STREAM_TYPE_COUNT];
  siera_timer_t flush_timer;
} siera_ds_t;

int siera_ds_init(siera_ds_t* ds,
  const siera_ds_stream_binding_t* streams,
  size_t stream_count,
  siera_timer_mgr_t* timers,
  uint32_t flush_interval_ms);
int siera_ds_read(const siera_ds_t* ds, siera_dsk_t key, void* out);
int siera_ds_write(siera_ds_t* ds, siera_dsk_t key, const void* in);
void siera_ds_deinit(siera_ds_t* ds);

int siera_ds_subscribe_all(siera_ds_t* ds, siera_event_sub_t* sub);
int siera_ds_unsubscribe_all(siera_ds_t* ds, siera_event_sub_t* sub);

const char* siera_ds_key_name(siera_dsk_t key);
size_t siera_ds_key_size(siera_dsk_t key);
siera_ds_stream_type_t siera_ds_key_stream_type(siera_dsk_t key);

#endif
