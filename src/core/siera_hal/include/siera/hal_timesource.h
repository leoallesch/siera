#ifndef SIERA_HAL_TIMESOURCE_H
#define SIERA_HAL_TIMESOURCE_H

#include <stdint.h>

typedef struct siera_hal_timesource_t {
    uint32_t (*get_ticks)(struct siera_hal_timesource_t *self);
} siera_hal_timesource_t;

static inline uint32_t siera_hal_timesource_get_ticks(siera_hal_timesource_t *self)
{
    return self->get_ticks(self);
}

#endif /* SIERA_HAL_TIMESOURCE_H */
