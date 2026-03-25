#ifndef SIERA_DS_KEYS_H
#define SIERA_DS_KEYS_H

#include <stdint.h>

/*
 * siera_dsk_t is generated from the user-supplied SIERA_DS_KEYS(KEY) macro
 * table defined in siera_ds_conf.h.
 *
 * To use siera_ds in your project:
 *   1. Copy siera_ds_conf.h.sample to your project as siera_ds_conf.h
 *   2. Define your keys in the SIERA_DS_KEYS(KEY) macro table
 *   3. Add the directory containing siera_ds_conf.h to your include path
 *
 * If no siera_ds_conf.h is found, the library falls back to a single dummy
 * key so it compiles without any user configuration.
 */

#if __has_include("siera_ds_conf.h")
#  include "siera_ds_conf.h"
#  ifndef SIERA_DS_KEYS
#    error "siera_ds_conf.h was found but does not define SIERA_DS_KEYS(KEY). " \
           "Check that your config file defines: #define SIERA_DS_KEYS(KEY) KEY(...)"
#  endif
#else
#define SIERA_DS_KEYS(KEY) \
    KEY(_DUMMY, uint8_t, SIERA_DS_RAM, 0, SIERA_DS_NONE)
#endif

enum {
#define _DS_KEY_ENUM(name, type, st, dv, fl) DSK_##name,
    SIERA_DS_KEYS(_DS_KEY_ENUM)
#undef _DS_KEY_ENUM
    DSK_COUNT
};

#endif
