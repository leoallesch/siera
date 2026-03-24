#ifndef SIERA_VIEW_MGR_H
#define SIERA_VIEW_MGR_H

#include "siera/view.h"
#include "siera/hal_display.h"

typedef struct {
  siera_hal_display_t *display;
  siera_view_t        *active;
} siera_view_mgr_t;

void siera_view_mgr_init(siera_view_mgr_t *mgr, siera_hal_display_t *display);
void siera_view_mgr_set(siera_view_mgr_t *mgr, siera_view_t *view);

#endif /* SIERA_VIEW_MGR_H */
