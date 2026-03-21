#pragma once

#include <stdbool.h>
#include <stdint.h>

enum {
  GPIO_DIRECTION_INPUT = 0,
  GPIO_DIRECTION_OUTPUT,
};
typedef uint8_t gpio_direction_t;

enum {
  GPIO_LEVEL_LOW = 0,
  GPIO_LEVEL_HIGH,
};
typedef uint8_t gpio_level_t;

typedef struct i_gpio_t {
  /**
   * @brief Set the direction of a GPIO pin.
   *
   * @param self       Pointer to the GPIO instance.
   * @param direction  GPIO_DIRECTION_INPUT or GPIO_DIRECTION_OUTPUT.
   */
  void (*set_direction)(struct i_gpio_t* self, gpio_direction_t direction);

  /**
   * @brief Write a logic level to a GPIO output.
   *
   * @param self   Pointer to the GPIO instance.
   * @param level  GPIO_LEVEL_LOW or GPIO_LEVEL_HIGH.
   */
  void (*write)(struct i_gpio_t* self, gpio_level_t level);

  /**
   * @brief Read the current logic level of a GPIO pin.
   *
   * @param self  Pointer to the GPIO instance.
   * @return true if the pin is high; false if low.
   */
  bool (*read)(struct i_gpio_t* self);

} i_gpio_t;

static inline void gpio_set_direction(i_gpio_t* self, gpio_direction_t direction)
{
  self->set_direction(self, direction);
}

static inline void gpio_write(i_gpio_t* self, gpio_level_t level)
{
  self->write(self, level);
}

static inline bool gpio_read(i_gpio_t* self)
{
  return self->read(self);
}