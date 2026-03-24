#ifndef SIERA_TIMER_H
#define SIERA_TIMER_H

#include <stdbool.h>
#include <stdint.h>
#include "siera/hal_timesource.h"
#include "siera/slist.h"

typedef void (*siera_timer_cb_fn)(void* ctx);

typedef struct {
  siera_slist_node_t node;
  siera_timer_cb_fn callback;
  void* ctx;
  uint32_t interval_ms;
  uint32_t expiry_ms;
  bool repeat;
  bool active;
} siera_timer_t;

typedef struct {
  siera_slist_head_t timers;
  siera_hal_timesource_t* timesource;
} siera_timer_mgr_t;

void siera_timer_mgr_init(siera_timer_mgr_t* mgr, siera_hal_timesource_t* timesource);
int  siera_timer_start(siera_timer_mgr_t* mgr, siera_timer_t* t, siera_timer_cb_fn cb, void* ctx, uint32_t interval_ms, bool repeat);
int  siera_timer_stop(siera_timer_mgr_t* mgr, siera_timer_t* t);
int  siera_timer_restart(siera_timer_mgr_t* mgr, siera_timer_t* t);
bool siera_timer_is_active(const siera_timer_t* t);
void siera_timer_tick(siera_timer_mgr_t* mgr);

#endif
