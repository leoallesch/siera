#ifndef SIERA_VIEW_MGR_H
#define SIERA_VIEW_MGR_H

#include "siera/ds.h"
#include "siera/dsk.h"
#include "siera/event.h"
#include "siera/hal_display.h"
#include "siera/view.h"

typedef struct {
  siera_hal_display_t* display;
  siera_view_t* active;
  i_siera_ds_t* ds;
  siera_event_sub_t sub;
  siera_dsk_t current_view_key;
} siera_view_mgr_t;

void siera_view_mgr_init(
  siera_view_mgr_t* mgr,
  i_siera_ds_t* ds,
  siera_hal_display_t* display,
  siera_dsk_t current_view_key);

#endif /* SIERA_VIEW_MGR_H */
