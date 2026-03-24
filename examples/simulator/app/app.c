#include "app.h"
#include "counter_view.h"

#include "siera/common.h"
#include "siera/event.h"
#include "siera/fsm.h"
#include "siera/view_mgr.h"

#include <stddef.h>

/* ── Presenter ───────────────────────────────────────────────────────────── */

typedef struct {
  siera_fsm_t fsm;
  siera_view_mgr_t view_mgr;
  siera_ds_t* ds;
  siera_timer_mgr_t* timers;
  siera_timer_t tick_timer;
  counter_view_t counter_view;
  siera_event_sub_t ds_sub;
} app_presenter_t;

static app_presenter_t g_presenter;

/* ── Helpers ─────────────────────────────────────────────────────────────── */

static app_presenter_t* p_from_fsm(siera_fsm_t* fsm)
{
  return SIERA_CONTAINER_OF(fsm, app_presenter_t, fsm);
}

/* ── Forward declarations ────────────────────────────────────────────────── */

static void state_counting(siera_fsm_t* fsm, siera_fsm_signal_t signal, const void* data);

/* ── DS change handler ───────────────────────────────────────────────────── */

static void on_ds_change(const void* args, void* ctx)
{
  app_presenter_t* p = (app_presenter_t*)ctx;
  const siera_ds_on_change_t* change = (const siera_ds_on_change_t*)args;

  if(change->key == SIERA_DS_KEY_COUNTER) {
    uint32_t value = *(const uint32_t*)change->data;
    counter_view_update(&p->counter_view, value);
  }
}

/* ── Timer callback ──────────────────────────────────────────────────────── */

static void on_tick(void* ctx)
{
  app_presenter_t* p = (app_presenter_t*)ctx;
  uint32_t counter;
  siera_ds_read(p->ds, SIERA_DS_KEY_COUNTER, &counter);
  counter++;
  siera_ds_write(p->ds, SIERA_DS_KEY_COUNTER, &counter);
}

/* ── FSM state ───────────────────────────────────────────────────────────── */

static void state_counting(siera_fsm_t* fsm, siera_fsm_signal_t signal, const void* data)
{
  (void)data;
  app_presenter_t* p = p_from_fsm(fsm);

  switch(signal) {
    case SIERA_FSM_SIGNAL_ENTER:
      siera_view_mgr_set(&p->view_mgr, &p->counter_view.base);
      siera_event_sub_init(&p->ds_sub, on_ds_change, p);
      siera_ds_subscribe_all(p->ds, &p->ds_sub);
      siera_timer_start(p->timers, &p->tick_timer, on_tick, p, 500, true);
      break;

    case SIERA_FSM_SIGNAL_EXIT:
      siera_timer_stop(p->timers, &p->tick_timer);
      siera_ds_unsubscribe_all(p->ds, &p->ds_sub);
      siera_view_mgr_set(&p->view_mgr, NULL);
      break;

    default:
      break;
  }
}

/* ── Public init ─────────────────────────────────────────────────────────── */

void app_init(siera_ds_t* ds, siera_timer_mgr_t* timers, siera_hal_display_t* display)
{
  g_presenter.ds = ds;
  g_presenter.timers = timers;

  counter_view_init(&g_presenter.counter_view);
  siera_view_mgr_init(&g_presenter.view_mgr, display);

  siera_fsm_init(&g_presenter.fsm, state_counting);
}
