#pragma once

#include <stdint.h>

typedef struct i_analog_input_t {
  /**
   * @brief Read a raw ADC sample from the given channel.
   *
   * @param self     Pointer to the analog input instance.
   * @return Raw conversion result (0 to 2^resolution_bits - 1, typically 0–4095 for 12-bit).
   */
  uint16_t (*read)(struct i_analog_input_t* self);

} i_analog_input_t;

static inline uint16_t analog_input_read(i_analog_input_t* self)
{
  return self->read(self);
}