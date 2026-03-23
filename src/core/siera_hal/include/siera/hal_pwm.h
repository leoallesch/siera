#ifndef SIERA_HAL_PWM_H
#define SIERA_HAL_PWM_H

#include <stdint.h>

typedef struct siera_hal_pwm_t {
    int (*set_duty)(struct siera_hal_pwm_t *self, uint32_t channel, uint32_t duty_permil);
    int (*enable)(struct siera_hal_pwm_t *self, uint32_t channel);
    int (*disable)(struct siera_hal_pwm_t *self, uint32_t channel);
} siera_hal_pwm_t;

/* duty_permil: 0–1000 (tenths of a percent, e.g. 500 = 50.0%) */
static inline int siera_pwm_set_duty(siera_hal_pwm_t *self, uint32_t channel,
                                     uint32_t duty_permil)
{
    return self->set_duty(self, channel, duty_permil);
}

static inline int siera_pwm_enable(siera_hal_pwm_t *self, uint32_t channel)
{
    return self->enable(self, channel);
}

static inline int siera_pwm_disable(siera_hal_pwm_t *self, uint32_t channel)
{
    return self->disable(self, channel);
}

#endif /* SIERA_HAL_PWM_H */
