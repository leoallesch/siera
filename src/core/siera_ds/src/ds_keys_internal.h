#ifndef SIERA_DS_KEYS_INTERNAL_H
#define SIERA_DS_KEYS_INTERNAL_H

#include "siera/ds_stream.h"
#include "siera/common.h"
#include <stdbool.h>
#include <string.h>

typedef enum {
    SIERA_DS_NONE     = 0,
    SIERA_DS_READONLY = (1 << 0),
    SIERA_DS_SILENT   = (1 << 1),
} siera_ds_flags_t;

/* ── Cache struct (layout driven by SIERA_DS_KEYS table) ────────────────── */

#define _DS_EXPAND_STORAGE(name, type, st, dv, fl) \
    uint8_t name[sizeof(type)];

typedef struct siera_ds_cache_t {
    SIERA_DS_KEYS(_DS_EXPAND_STORAGE)
} siera_ds_cache_t;

#undef _DS_EXPAND_STORAGE

/* ── Apply defaults ─────────────────────────────────────────────────────── */

#define _DS_EXPAND_DEFAULT(name, type, st, dv, fl)                           \
  {                                                                           \
    type _tmp = (type){dv};                                                   \
    memcpy((uint8_t*)cache + SIERA_OFFSET_OF(siera_ds_cache_t, name),        \
           &_tmp, sizeof(type));                                              \
  }

static inline void _siera_ds_apply_defaults(siera_ds_cache_t *cache) {
    SIERA_DS_KEYS(_DS_EXPAND_DEFAULT)
}

#undef _DS_EXPAND_DEFAULT

/* ── Entry metadata ─────────────────────────────────────────────────────── */

typedef struct {
    const char             *name;
    siera_ds_stream_type_t  stream_type;
    size_t                  size;
    size_t                  offset;
    siera_ds_flags_t        flags;
} siera_ds_entry_t;

extern const siera_ds_entry_t _siera_ds_entry_table[];

#ifndef SIERA_DS_MAX_VALUE_SIZE
#define SIERA_DS_MAX_VALUE_SIZE 8
#endif

#endif /* SIERA_DS_KEYS_INTERNAL_H */
