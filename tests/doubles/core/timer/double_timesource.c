#include "double_timesource.h"

static timesource_ticks_t get_ticks(i_timesource_t* instance)
{
  double_timesource_t* self = (double_timesource_t*)instance;
  return self->ticks;
}

void double_timesource_init(double_timesource_t* self)
{
  self->interface.get_ticks = get_ticks;
  self->ticks = 0;
}

void double_timesource_set_ticks(double_timesource_t* self, timesource_ticks_t ticks)
{
  self->ticks = ticks;
}

void double_timesource_advance_ticks(double_timesource_t* self, timesource_ticks_t ticks)
{
  self->ticks += ticks;
}
