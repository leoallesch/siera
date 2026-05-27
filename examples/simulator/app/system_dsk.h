#include <stdint.h>
#include "siera/key_event.h" /* siera_key_event_data_t for KEY_EVENT entry */

#define DSK_RAM_KEYS(KEY)                \
  KEY(COUNTER, uint32_t)                 \
  KEY(BTN_PAUSE, bool)                   \
  KEY(BTN_RESET, bool)                   \
  KEY(KEY_EVENT, siera_key_event_data_t) \
  KEY(CURRENT_VIEW, siera_view_t*)

#define EXPAND_AS_ENUM(name, type) DSK_##name,

enum {
  DSK_RAM_BASE,
  DSK_RAM_KEYS(EXPAND_AS_ENUM)
    DSK_BSP_BASE,
  DSK_COUNT
};
