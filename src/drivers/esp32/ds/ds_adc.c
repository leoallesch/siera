#include "siera/esp32_ds_adc.h"

#include <stddef.h>

static siera_esp32_ds_adc_t* self_of(i_siera_ds_t* iface)
{
  return (siera_esp32_ds_adc_t*)iface;
}

static const siera_esp32_ds_adc_channel_t* find_channel(const siera_esp32_ds_adc_t* drv,
  siera_dsk_t key)
{
  for(size_t i = 0; i < drv->channel_count; i++) {
    if(drv->channels[i].key == key)
      return &drv->channels[i];
  }
  return NULL;
}

static int _read(i_siera_ds_t* iface, siera_dsk_t key, void* buf, size_t size)
{
  (void)size;
  siera_esp32_ds_adc_t* drv = self_of(iface);
  const siera_esp32_ds_adc_channel_t* ch = find_channel(drv, key);
  if(!ch)
    return -1;

  return siera_adc_read(drv->hal, ch->channel, (siera_adc_counts_t*)buf);
}

static int _write(i_siera_ds_t* iface, siera_dsk_t key, const void* buf, size_t size)
{
  (void)iface;
  (void)key;
  (void)buf;
  (void)size;
  return -1; /* ADC is read-only */
}

static bool _contains(i_siera_ds_t* iface, siera_dsk_t key)
{
  return find_channel(self_of(iface), key) != NULL;
}

static size_t _size(i_siera_ds_t* iface, siera_dsk_t key)
{
  (void)iface;
  (void)key;
  return sizeof(siera_adc_counts_t);
}

static siera_event_t* _on_change(i_siera_ds_t* iface)
{
  return &self_of(iface)->on_change;
}

static const i_siera_ds_stream_api_t _api = {
  .read = _read,
  .write = _write,
  .contains = _contains,
  .size = _size,
  .on_change = _on_change,
};

void siera_esp32_ds_adc_init(siera_esp32_ds_adc_t* self,
  siera_hal_adc_t* hal,
  const siera_esp32_ds_adc_channel_t* channels,
  size_t channel_count)
{
  self->interface.api = &_api;
  self->hal = hal;
  self->channels = channels;
  self->channel_count = channel_count;
  siera_event_init(&self->on_change);
}
