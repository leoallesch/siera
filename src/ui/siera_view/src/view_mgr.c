#include "siera/view_mgr.h"

#include <stddef.h>

void siera_view_mgr_init(siera_view_mgr_t *mgr, siera_hal_display_t *display)
{
  mgr->display = display;
  mgr->active  = NULL;
}

void siera_view_mgr_set(siera_view_mgr_t *mgr, siera_view_t *view)
{
  if(mgr->active != NULL)
    siera_view_unload(mgr->active);

  mgr->active = view;

  if(mgr->active != NULL)
    siera_view_load(mgr->active, siera_hal_display_get_canvas(mgr->display));
}
