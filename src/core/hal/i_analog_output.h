#pragma once

#include <stdint.h>

typedef struct i_analog_output_t {
  /**
   * @brief Write a raw DAC value.
   *
   * @param self     Pointer to the analog output instance.
   * @param value    Raw output value (0 to 2^resolution_bits - 1, typically 0–4095 for 12-bit).
   */
  void (*write)(struct i_analog_output_t* self, uint16_t value);
} i_analog_output_t;

static inline void analog_output_write(i_analog_output_t* self, uint16_t value)
{
  self->write(self, value);
}
