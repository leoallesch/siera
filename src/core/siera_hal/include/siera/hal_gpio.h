#ifndef SIERA_HAL_GPIO_H
#define SIERA_HAL_GPIO_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    SIERA_GPIO_DIR_INPUT  = 0,
    SIERA_GPIO_DIR_OUTPUT = 1,
} siera_gpio_dir_t;

typedef enum {
    SIERA_GPIO_PULL_NONE = 0,
    SIERA_GPIO_PULL_UP   = 1,
    SIERA_GPIO_PULL_DOWN = 2,
} siera_gpio_pull_t;

typedef struct siera_hal_gpio_t {
    int  (*configure)(struct siera_hal_gpio_t *self, uint32_t pin,
                      siera_gpio_dir_t dir, siera_gpio_pull_t pull);
    int  (*write)(struct siera_hal_gpio_t *self, uint32_t pin, bool value);
    int  (*read)(struct siera_hal_gpio_t *self, uint32_t pin, bool *value);
} siera_hal_gpio_t;

static inline int siera_gpio_configure(siera_hal_gpio_t *self, uint32_t pin,
                                       siera_gpio_dir_t dir, siera_gpio_pull_t pull)
{
    return self->configure(self, pin, dir, pull);
}

static inline int siera_gpio_write(siera_hal_gpio_t *self, uint32_t pin, bool value)
{
    return self->write(self, pin, value);
}

static inline int siera_gpio_read(siera_hal_gpio_t *self, uint32_t pin, bool *value)
{
    return self->read(self, pin, value);
}

#endif /* SIERA_HAL_GPIO_H */
