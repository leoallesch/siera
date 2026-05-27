#include "siera/event.h"
#include "siera/view_mgr.h"

#include <stddef.h>

static void set(siera_view_mgr_t* mgr, siera_view_t* view)
{
  if(mgr->active != NULL)
    siera_view_unload(mgr->active);

  mgr->active = view;

  if(mgr->active != NULL)
    siera_view_load(mgr->active, siera_hal_display_get_canvas(mgr->display));
}

static void on_change(void* ctx, const void* data)
{
  siera_view_mgr_t* mgr = (siera_view_mgr_t*)ctx;
  const siera_ds_on_change_args_t* args = (const siera_ds_on_change_args_t*)data;

  if(args->key == mgr->current_view_key) {
    siera_view_t* view = *(siera_view_t**)args->data;
    set(mgr, view);
  }
}

void siera_view_mgr_init(
  siera_view_mgr_t* mgr,
  i_siera_ds_t* ds,
  siera_hal_display_t* display,
  siera_dsk_t current_view_key)
{
  mgr->ds = ds;
  mgr->display = display;
  mgr->active = NULL;
  mgr->current_view_key = current_view_key;

  siera_event_sub_init(&mgr->sub, on_change, mgr);
  siera_ds_subscribe_all(mgr->ds, &mgr->sub);
}
