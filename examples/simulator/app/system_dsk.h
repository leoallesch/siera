#ifndef SYSTEM_DSK_H
#define SYSTEM_DSK_H

#include <stdint.h>
#include "siera/common.h"
#include "siera/key_event.h"
#include "siera/view.h"

/* ── Category filters ─────────────────────────────────────────────────────────
 * INCLUDE_<key_tag>_<target>(x) expands to `x` when a key tagged <key_tag>
 * should participate in <target>'s code-gen, and to empty otherwise. */

#define INCLUDE_STORAGE_RAM_RAM(x) x
#define INCLUDE_STORAGE_NVS_RAM(x)
#define INCLUDE_STORAGE_BSP_RAM(x)

#define INCLUDE_STORAGE_RAM_NVS(x)
#define INCLUDE_STORAGE_NVS_NVS(x) x
#define INCLUDE_STORAGE_BSP_NVS(x)

#define INCLUDE_STORAGE_RAM_BSP(x)
#define INCLUDE_STORAGE_NVS_BSP(x)
#define INCLUDE_STORAGE_BSP_BSP(x) x

/* ── Key schema ──────────────────────────────────────────────────────────────
 *   KEY(name, type, storage)  — storage ∈ { RAM, NVS, BSP } */

#define DSK_ENTRIES(KEY)                          \
  KEY(DSK_COUNTER, uint32_t, NVS)                 \
  KEY(DSK_KEY_EVENT, siera_key_event_data_t, RAM) \
  KEY(DSK_CURRENT_VIEW, siera_view_t*, RAM)       \
  KEY(DSK_BTN_PAUSE, bool, BSP)                   \
  KEY(DSK_BTN_RESET, bool, BSP)

/* ── Expanders (consumed by main.c / bsp code) ──────────────────────────────── */

/* Per-category enum emitters — produce a key only when its tag matches the
   target. This lets us emit keys in category order even though the schema
   lists them in author order. */
#define EXPAND_AS_RAM_ENUM(name, type, storage) SIERA_CONCAT(INCLUDE_STORAGE_RAM_, storage)(name COMMA)
#define EXPAND_AS_NVS_ENUM(name, type, storage) SIERA_CONCAT(INCLUDE_STORAGE_NVS_, storage)(name COMMA)
#define EXPAND_AS_BSP_ENUM(name, type, storage) SIERA_CONCAT(INCLUDE_STORAGE_BSP_, storage)(name COMMA)

#define EXPAND_AS_RAM_STORAGE(name, type, storage) \
  SIERA_CONCAT(INCLUDE_STORAGE_RAM_, storage)(type name;)

#define EXPAND_AS_RAM_CONFIG(name, type, storage) \
  SIERA_CONCAT(INCLUDE_STORAGE_RAM_, storage)(    \
    { offsetof(ds_ram_storage_t, name) COMMA sizeof(type) } COMMA)

#define EXPAND_AS_NVS_REGISTER(name, type, storage) \
  SIERA_CONCAT(INCLUDE_STORAGE_NVS_, storage)(      \
    siera_sim_ds_nvs_register(&g_nvs COMMA name COMMA sizeof(type));)

/* Keys are assigned values in category order: RAM block first (indices
   0..DSK_RAM_END-1), then NVS, then BSP. Dense per-stream config arrays
   index directly by key value within the appropriate range. */
enum {
  DSK_ENTRIES(EXPAND_AS_RAM_ENUM)
    DSK_RAM_END,
  DSK_NVS_BASE = DSK_RAM_END,
  DSK_ENTRIES(EXPAND_AS_NVS_ENUM)
    DSK_NVS_END,
  DSK_BSP_BASE = DSK_NVS_END,
  DSK_ENTRIES(EXPAND_AS_BSP_ENUM)
    DSK_COUNT,

  DSK_RAM_COUNT = DSK_RAM_END,
  DSK_NVS_COUNT = DSK_NVS_END - DSK_NVS_BASE,
  DSK_BSP_COUNT = DSK_COUNT - DSK_BSP_BASE,
};

#endif /* APP_SYSTEM_DSK_H */
