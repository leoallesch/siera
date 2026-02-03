#pragma once

#include "i_timesource.h"
#include "list.h"

#include <stdbool.h>

typedef void (*timer_callback_t)(void* context);

typedef struct
{
  i_timesource_t* timesource;
  timesource_ticks_t start_ticks;

  list_t timers;

} timer_controller_t;

typedef struct
{
  list_node_t node;
  timer_controller_t* controller;
  timer_callback_t callback;
  void* context;
  timesource_ticks_t interval_ticks;
  timesource_ticks_t next_expiration_ticks;
  bool repeating;
} timer_t;

void timer_controller_init(timer_controller_t* controller, i_timesource_t* timesource);
void timer_controller_run(timer_controller_t* controller);

void timer_start_one_shot(timer_t* timer, timer_controller_t* controller, timesource_ticks_t interval_ticks, timer_callback_t callback, void* context);
void timer_start_repeating(timer_t* timer, timer_controller_t* controller, timesource_ticks_t interval_ticks, timer_callback_t callback, void* context);
void timer_stop(timer_t* timer);

bool timer_is_active(timer_controller_t* controller, timer_t* timer);
