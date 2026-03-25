#include <stdint.h>
#include "siera/key_event.h"  /* siera_key_event_data_t for KEY_EVENT entry */

#define SIERA_DS_KEYS(KEY)                                                              \
  KEY(COUNTER,   uint32_t,              SIERA_DS_NVS, 100, SIERA_DS_NONE)             \
  KEY(BTN_PAUSE, bool,                  SIERA_DS_GPIO, 0,   SIERA_DS_NONE)             \
  KEY(BTN_RESET, bool,                  SIERA_DS_GPIO, 0,   SIERA_DS_NONE)             \
  KEY(KEY_EVENT, siera_key_event_data_t, SIERA_DS_RAM, 0, SIERA_DS_NONE)
