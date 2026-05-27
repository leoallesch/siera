#include "app.h"
#include "counter_view.h"

#include "siera/common.h"
#include "siera/event.h"
#include "siera/fsm.h"
#include "siera/key_manager.h"
#include "siera/view_mgr.h"

#include "system_dsk.h"

#include <stddef.h>

/* ── Presenter ───────────────────────────────────────────────────────────── */

typedef struct {
  siera_fsm_t fsm;
  siera_view_mgr_t view_mgr;
  i_siera_ds_t* ds;
  siera_timer_mgr_t* timers;
  siera_timer_t tick_timer;
  counter_view_t counter_view;
  siera_event_sub_t ds_sub;
  siera_key_manager_t key_mgr;
  bool paused;
} app_presenter_t;

static app_presenter_t g_presenter;

/* ── Helpers ─────────────────────────────────────────────────────────────── */

static app_presenter_t* p_from_fsm(siera_fsm_t* fsm)
{
  return SIERA_CONTAINER_OF(fsm, app_presenter_t, fsm);
}

/* ── Forward declarations ────────────────────────────────────────────────── */

static void state_counting(siera_fsm_t* fsm, siera_fsm_signal_t signal, const void* data);

/* ── Timer callback ──────────────────────────────────────────────────────── */

static void on_tick(void* ctx)
{
  app_presenter_t* p = (app_presenter_t*)ctx;
  uint32_t counter;
  siera_ds_read(p->ds, DSK_COUNTER, &counter);
  counter++;
  siera_ds_write(p->ds, DSK_COUNTER, &counter);
}

/* ── DS change handler ───────────────────────────────────────────────────── */

static void on_ds_change(void* ctx, const void* args)
{
  app_presenter_t* p = (app_presenter_t*)ctx;
  const siera_ds_on_change_args_t* change = (const siera_ds_on_change_args_t*)args;

  if(change->key == DSK_COUNTER) {
    counter_view_update(&p->counter_view, *(const uint32_t*)change->data);
  }
  else if(change->key == DSK_KEY_EVENT) {
    const siera_key_event_data_t* ke = (const siera_key_event_data_t*)change->data;
    if(ke->event != SIERA_KEY_EVENT_PRESSANDRELEASE)
      return;

    if(ke->key == DSK_BTN_PAUSE) {
      p->paused = !p->paused;
      if(p->paused)
        siera_timer_stop(p->timers, &p->tick_timer);
      else
        siera_timer_start(p->timers, &p->tick_timer, on_tick, p, 500, true);
    }
    else if(ke->key == DSK_BTN_RESET) {
      uint32_t zero = 0;
      siera_ds_write(p->ds, DSK_COUNTER, &zero);
    }
  }
}

/* ── FSM state ───────────────────────────────────────────────────────────── */

static void state_counting(siera_fsm_t* fsm, siera_fsm_signal_t signal, const void* data)
{
  (void)data;
  app_presenter_t* p = p_from_fsm(fsm);

  switch(signal) {
    case SIERA_FSM_SIGNAL_ENTER: {
      siera_view_t* view = &p->counter_view.base;
      siera_ds_write(p->ds, DSK_CURRENT_VIEW, &view);

      uint32_t counter;
      siera_ds_read(p->ds, DSK_COUNTER, &counter);
      counter_view_update(&p->counter_view, counter);

      siera_timer_start(p->timers, &p->tick_timer, on_tick, p, 500, true);
      break;
    }

    case SIERA_FSM_SIGNAL_EXIT: {
      siera_view_t* null_view = NULL;
      siera_timer_stop(p->timers, &p->tick_timer);
      siera_ds_write(p->ds, DSK_CURRENT_VIEW, &null_view);
      break;
    }

    default:
      break;
  }
}

static const siera_dsk_t btn_keys[] = {
  DSK_BTN_PAUSE,
  DSK_BTN_RESET,
};

/* ── Public init ─────────────────────────────────────────────────────────── */

void app_init(i_siera_ds_t* ds, siera_timer_mgr_t* timers, siera_hal_display_t* display)
{
  g_presenter.ds = ds;
  g_presenter.timers = timers;
  g_presenter.paused = false;

  counter_view_init(&g_presenter.counter_view);
  siera_view_mgr_init(&g_presenter.view_mgr, ds, display, DSK_CURRENT_VIEW);

  siera_key_manager_init(
    &g_presenter.key_mgr, g_presenter.ds, g_presenter.timers, DSK_KEY_EVENT, 500, btn_keys, SIERA_NUM_ELEMENTS(btn_keys));

  siera_event_sub_init(&g_presenter.ds_sub, on_ds_change, &g_presenter);
  siera_ds_subscribe_all(g_presenter.ds, &g_presenter.ds_sub);

  siera_fsm_init(&g_presenter.fsm, state_counting);
}
