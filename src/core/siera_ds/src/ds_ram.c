#include <string.h>
#include "siera/ds_ram.h"

static uint16_t offset(siera_ds_ram_t* instance, siera_dsk_t key)
{
  return instance->configs[key].offset;
}

static size_t size(i_siera_ds_t* interface, siera_dsk_t key)
{
  siera_ds_ram_t* instance = (siera_ds_ram_t*)interface;
  return instance->configs[key].size;
}

static void read(i_siera_ds_t* interface, siera_dsk_t key, void* buf)
{
  siera_ds_ram_t* instance = (siera_ds_ram_t*)interface;
  memcpy(buf, instance->storage + offset(instance, key), size(interface, key));
}

static void write(i_siera_ds_t* interface, siera_dsk_t key, const void* buf)
{
  siera_ds_ram_t* instance = (siera_ds_ram_t*)interface;
  uint8_t value_size = size(interface, key);
  void* location = (uint8_t*)instance->storage + offset(instance, key);

  if(memcmp(location, buf, value_size)) {
    memcpy(location, buf, value_size);

    siera_ds_on_change_args_t args = { key, buf };
    siera_event_publish(&instance->on_change, &args);
  }
}

static bool contains(i_siera_ds_t* interface, siera_dsk_t key)
{
  return key < ((siera_ds_ram_t*)interface)->count;
}

static siera_event_t* on_change(i_siera_ds_t* interface)
{
  siera_ds_ram_t* instance = (siera_ds_ram_t*)interface;
  return &instance->on_change;
}

static const i_siera_ds_api_t _ram_api = {
  .read = read,
  .write = write,
  .contains = contains,
  .size = size,
  .on_change = on_change,
};

void siera_ds_ram_init(
  siera_ds_ram_t* instance,
  const siera_ram_config_t* configs,
  uint16_t count,
  void* storage)
{
  instance->interface.api = &_ram_api;
  instance->configs = configs;
  instance->count = count;
  instance->storage = storage;

  siera_event_init(&instance->on_change);

  siera_dsk_t last_key = (siera_dsk_t)(instance->count - 1);
  uint16_t last_offset = offset(instance, last_key);
  uint8_t last_size = size(&instance->interface, last_key);
  memset(instance->storage, 0, last_offset + last_size);
}
