#include <stddef.h>
#include "siera/common.h"
#include "siera/timer.h"

void siera_timer_mgr_init(siera_timer_mgr_t* mgr, siera_hal_timesource_t* timesource)
{
  siera_slist_init(&mgr->timers);
  mgr->timesource = timesource;
}

int siera_timer_start(siera_timer_mgr_t* mgr, siera_timer_t* t, siera_timer_cb_fn cb, void* ctx, uint32_t interval_ms, bool repeat)
{
  if(!mgr || !t || !cb)
    return -1;
  siera_slist_node_init(&t->node);
  t->callback = cb;
  t->ctx = ctx;
  // Remove from list if already active
  siera_slist_remove(&mgr->timers, &t->node);
  t->interval_ms = interval_ms;
  t->repeat = repeat;
  t->expiry_ms = siera_hal_timesource_get_ticks(mgr->timesource) + interval_ms;
  t->active = true;
  siera_slist_push(&mgr->timers, &t->node);
  return 0;
}

int siera_timer_stop(siera_timer_mgr_t* mgr, siera_timer_t* t)
{
  if(!mgr || !t)
    return -1;
  siera_slist_remove(&mgr->timers, &t->node);
  t->active = false;
  return 0;
}

int siera_timer_restart(siera_timer_mgr_t* mgr, siera_timer_t* t)
{
  if(!mgr || !t)
    return -1;
  t->expiry_ms = siera_hal_timesource_get_ticks(mgr->timesource) + t->interval_ms;
  if(!t->active) {
    t->active = true;
    siera_slist_push(&mgr->timers, &t->node);
  }
  return 0;
}

bool siera_timer_is_active(const siera_timer_t* t)
{
  return t && t->active;
}

void siera_timer_tick(siera_timer_mgr_t* mgr)
{
  if(!mgr || !mgr->timesource)
    return;
  uint32_t now = siera_hal_timesource_get_ticks(mgr->timesource);

  siera_slist_node_t *node, *tmp;
  SIERA_SLIST_FOR_EACH_SAFE(&mgr->timers, node, tmp)
  {
    siera_timer_t* t = SIERA_CONTAINER_OF(node, siera_timer_t, node);
    if(!t->active)
      continue;
    // Wraparound-safe: (int32_t)(now - expiry) >= 0
    if((int32_t)(now - t->expiry_ms) < 0)
      continue;

    if(t->repeat) {
      // Drift-corrected: advance from last expiry, not from now
      t->expiry_ms += t->interval_ms;
    }
    else {
      siera_slist_remove(&mgr->timers, &t->node);
      t->active = false;
    }

    t->callback(t->ctx);
  }
}
