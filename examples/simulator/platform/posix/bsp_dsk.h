#include "siera/sim_ds_io.h"
#include "system_dsk.h"

/* Platform-specific binding for DS_BSP keys: each key maps to a sim widget.
 *   BSP_KEY(name, widget_type, label)
 *     name        — DSK_<name> must exist in system_dsk.h with category DS_BSP
 *     widget_type — SIERA_SIM_WIDGET_<widget_type>
 *     label       — bare identifier; stringified for the panel label */

#define DSK_BSP_ENTRIES(BSP_KEY)         \
  BSP_KEY(DSK_BTN_PAUSE, BUTTON, Pause)      \
  BSP_KEY(DSK_BTN_RESET, BUTTON, Reset)

#define EXPAND_AS_WIDGET_CONFIG(name, type, label) \
  { name, SIERA_SIM_WIDGET_##type, #label },
