#ifndef APP_H
#define APP_H

#include "siera/ds.h"
#include "siera/hal_display.h"
#include "siera/timer.h"

void app_init(i_siera_ds_t* ds, siera_timer_mgr_t* timers, siera_hal_display_t* display);

#endif /* APP_H */
