#ifndef SIERA_ESP32_PWM_H
#define SIERA_ESP32_PWM_H

#include "siera/hal_pwm.h"

#include "driver/ledc.h"

/* One ESP32 PWM driver instance covers up to LEDC_CHANNEL_MAX channels.
   All channels share the same timer (LEDC_TIMER_0, 10-bit, 5 kHz). */
siera_hal_pwm_t *siera_esp32_pwm_init(ledc_mode_t speed_mode);

#endif /* SIERA_ESP32_PWM_H */
