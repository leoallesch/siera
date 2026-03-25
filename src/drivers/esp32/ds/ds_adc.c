#include "siera/esp32_ds_adc.h"

#include <stddef.h>

static const siera_esp32_ds_adc_channel_t* find_channel(const siera_esp32_ds_adc_t* drv,
  siera_dsk_t key)
{
  for(size_t i = 0; i < drv->channel_count; i++) {
    if(drv->channels[i].key == key)
      return &drv->channels[i];
  }
  return NULL;
}

static int _read(void* ctx, siera_dsk_t key, void* buf, size_t size)
{
  (void)size;
  siera_esp32_ds_adc_t* drv = (siera_esp32_ds_adc_t*)ctx;
  const siera_esp32_ds_adc_channel_t* ch = find_channel(drv, key);
  if(!ch)
    return -1;

  return siera_adc_read(drv->hal, ch->channel, (siera_adc_counts_t*)buf);
}

static int _write(void* ctx, siera_dsk_t key, const void* buf, size_t size)
{
  (void)ctx;
  (void)key;
  (void)buf;
  (void)size;
  return -1; /* ADC is read-only */
}

static const siera_ds_stream_api_t _api = { _read, _write };

void siera_esp32_ds_adc_init(siera_esp32_ds_adc_t* self,
  siera_hal_adc_t* hal,
  const siera_esp32_ds_adc_channel_t* channels,
  size_t channel_count)
{
  self->hal = hal;
  self->channels = channels;
  self->channel_count = channel_count;
  self->stream.api = &_api;
  self->stream.ctx = self;
}
