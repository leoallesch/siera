#include "siera/esp32_adc.h"

typedef struct {
  siera_hal_adc_t           interface;
  adc_oneshot_unit_handle_t handle;
} siera_esp32_adc_t;

static siera_esp32_adc_t s_adc;

static int _read(siera_hal_adc_t* self, uint32_t channel, siera_adc_counts_t* value)
{
  siera_esp32_adc_t* drv = (siera_esp32_adc_t*)self;
  int raw = 0;
  esp_err_t err = adc_oneshot_read(drv->handle, (adc_channel_t)channel, &raw);
  if(err != ESP_OK)
    return -1;
  *value = (siera_adc_counts_t)raw;
  return 0;
}

siera_hal_adc_t* siera_esp32_adc_init(adc_unit_t unit)
{
  adc_oneshot_unit_init_cfg_t cfg = { .unit_id = unit };
  adc_oneshot_new_unit(&cfg, &s_adc.handle);
  s_adc.interface.read = _read;
  return &s_adc.interface;
}
