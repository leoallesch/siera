#pragma once

#include "i_timesource.h"
#include "list.h"

#include <stdbool.h>

typedef void (*timer_callback_t)(void* context);

typedef struct
{
  i_timesource_t* timesource;
  timesource_ticks_t current_ticks;
  list_t timers;
} s_timer_controller_t;

typedef struct
{
  list_node_t node;
  s_timer_controller_t* controller;
  timer_callback_t callback;
  void* context;
  timesource_ticks_t interval_ticks;
  timesource_ticks_t next_expiration_ticks;
  bool repeating;
} s_timer_t;

void timer_controller_init(s_timer_controller_t* controller, i_timesource_t* timesource);
timesource_ticks_t timer_controller_run(s_timer_controller_t* controller);
void timer_start_one_shot(s_timer_t* timer, s_timer_controller_t* controller, timesource_ticks_t interval_ticks, timer_callback_t callback, void* context);
void timer_start_repeating(s_timer_t* timer, s_timer_controller_t* controller, timesource_ticks_t interval_ticks, timer_callback_t callback, void* context);
void timer_stop(s_timer_t* timer);
bool timer_is_active(s_timer_controller_t* controller, s_timer_t* timer);
