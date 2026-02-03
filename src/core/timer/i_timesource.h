#pragma once

#include <stdint.h>

typedef uint32_t timesource_ticks_t;

typedef struct i_timesource_t {
  /**
   * @brief Get the current time in milliseconds.
   *
   * @param self Pointer to the timesource instance.
   * @return uint32_t Current time in milliseconds.
   */
  timesource_ticks_t (*get_ticks)(struct i_timesource_t* instance);
} i_timesource_t;
