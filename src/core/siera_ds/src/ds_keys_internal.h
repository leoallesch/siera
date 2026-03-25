#ifndef SIERA_DS_KEYS_INTERNAL_H
#define SIERA_DS_KEYS_INTERNAL_H

#include <stdbool.h>
#include <string.h>

#include "siera/common.h"
#include "siera/ds_keys.h"

/* ── Cache struct (layout driven by SIERA_DS_KEYS table) ────────────────── */

#define _DS_EXPAND_STORAGE(name, type, st, dv, fl) \
  uint8_t name[sizeof(type)];

typedef struct siera_ds_cache_t {
  SIERA_DS_KEYS(_DS_EXPAND_STORAGE)
} siera_ds_cache_t;

#undef _DS_EXPAND_STORAGE

/* ── Apply defaults ─────────────────────────────────────────────────────── */

#define _DS_EXPAND_DEFAULT(name, type, st, dv, fl)                    \
  {                                                                   \
    type _tmp = (type){ dv };                                         \
    memcpy((uint8_t*)cache + SIERA_OFFSET_OF(siera_ds_cache_t, name), \
      &_tmp,                                                          \
      sizeof(type));                                                  \
  }

static inline void _siera_ds_apply_defaults(siera_ds_cache_t* cache){
  SIERA_DS_KEYS(_DS_EXPAND_DEFAULT)
}

#undef _DS_EXPAND_DEFAULT

#ifndef SIERA_DS_MAX_VALUE_SIZE
#define SIERA_DS_MAX_VALUE_SIZE 8
#endif

#endif /* SIERA_DS_KEYS_INTERNAL_H */
