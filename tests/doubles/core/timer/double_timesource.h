#pragma once

#include "i_timesource.h"

typedef struct
{
  i_timesource_t interface;
  timesource_ticks_t ticks;
} double_timesource_t;

void double_timesource_init(double_timesource_t* self);
void double_timesource_set_ticks(double_timesource_t* self, timesource_ticks_t ticks);
void double_timesource_advance_ticks(double_timesource_t* self, timesource_ticks_t ticks);
