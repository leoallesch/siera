#include "siera/esp32_gpio.h"

#include "driver/gpio.h"

static siera_hal_gpio_t s_gpio;

static int _configure(siera_hal_gpio_t* self, uint32_t pin, siera_gpio_dir_t dir, siera_gpio_pull_t pull)
{
  (void)self;
  gpio_config_t cfg = {
    .pin_bit_mask = (1ULL << pin),
    .mode = (dir == SIERA_GPIO_DIR_OUTPUT) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT,
    .pull_up_en = (pull == SIERA_GPIO_PULL_UP) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
    .pull_down_en = (pull == SIERA_GPIO_PULL_DOWN) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  return gpio_config(&cfg) == ESP_OK ? 0 : -1;
}

static int _write(siera_hal_gpio_t* self, uint32_t pin, bool value)
{
  (void)self;
  gpio_set_level((gpio_num_t)pin, value ? 1 : 0);
  return 0;
}

static int _read(siera_hal_gpio_t* self, uint32_t pin, bool* value)
{
  (void)self;
  *value = gpio_get_level((gpio_num_t)pin) != 0;
  return 0;
}

siera_hal_gpio_t* siera_esp32_gpio_init(void)
{
  s_gpio.configure = _configure;
  s_gpio.write = _write;
  s_gpio.read = _read;
  return &s_gpio;
}
