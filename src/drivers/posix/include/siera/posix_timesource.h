#ifndef SIERA_POSIX_TIMESOURCE_H
#define SIERA_POSIX_TIMESOURCE_H

#include "siera/hal_timesource.h"

typedef struct {
  siera_timesource_t interface;
} posix_timesource_t;

void posix_timesource_init(posix_timesource_t *self);

#endif /* SIERA_POSIX_TIMESOURCE_H */
