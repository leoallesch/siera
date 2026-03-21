#pragma once

#include <stdint.h>

typedef struct i_pwm_t {
  /**
   * @brief Set the duty cycle for a PWM channel.
   *
   * @param self     Pointer to the PWM instance.
   * @param channel  PWM channel index.
   * @param duty     Duty cycle value (0 to 2^resolution_bits - 1).
   */
  void (*set_duty)(struct i_pwm_t* self, uint8_t channel, uint32_t duty);

  /**
   * @brief Set the carrier frequency for a PWM channel.
   *
   * @param self          Pointer to the PWM instance.
   * @param channel       PWM channel index.
   * @param frequency_hz  Carrier frequency in hertz.
   */
  void (*set_frequency)(struct i_pwm_t* self, uint8_t channel, uint32_t frequency_hz);
} i_pwm_t;

static inline void pwm_set_duty(i_pwm_t* self, uint8_t channel, uint32_t duty)
{
  self->set_duty(self, channel, duty);
}

static inline void pwm_set_frequency(i_pwm_t* self, uint8_t channel, uint32_t frequency_hz)
{
  self->set_frequency(self, channel, frequency_hz);
}
