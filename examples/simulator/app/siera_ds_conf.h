#include <stdint.h>

#define SIERA_DS_KEYS(KEY)                               \
  KEY(COUNTER, uint32_t, SIERA_DS_RAM, 0, SIERA_DS_NONE) \
  KEY(BUTTON_1, bool, SIERA_DS_RAM, 0, SIERA_DS_NONE)    \
  KEY(BUTTON_2, bool, SIERA_DS_RAM, 0, SIERA_DS_NONE)    \
  KEY(BUTTON_3, bool, SIERA_DS_RAM, 0, SIERA_DS_NONE)    \
  KEY(BUTTON_4, bool, SIERA_DS_RAM, 0, SIERA_DS_NONE)    \
  KEY(BUTTON_5, bool, SIERA_DS_RAM, 0, SIERA_DS_NONE)
