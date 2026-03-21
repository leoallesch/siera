#include <stdio.h>

#include "i_datastream.h"
#include "nvs_datastream.h"

static bool contains(i_datastream_t* interface, datastream_key_t key)
{
  nvs_datastream_t* instance = (nvs_datastream_t*)interface;
  return key < instance->config->count && instance->config->entries[key].size > 0;
}

static uint8_t size(i_datastream_t* interface, datastream_key_t key)
{
  if(contains(interface, key)) {
    nvs_datastream_t* instance = (nvs_datastream_t*)interface;
    return instance->config->entries[key].size;
  }
  return 0;
}

static void read(i_datastream_t* interface, datastream_key_t key, void* out)
{
  if(!contains(interface, key))
    return;

  nvs_datastream_t* instance = (nvs_datastream_t*)interface;
  const nvs_datastream_entry_t* entry = &instance->config->entries[key];

  char key_name[8];
  snprintf(key_name, sizeof(key_name), "k%u", (unsigned)key);
  flash_kv_read(instance->flash_kv, key_name, out, entry->size);
}

static void write(i_datastream_t* interface, datastream_key_t key, const void* data)
{
  if(!contains(interface, key))
    return;

  nvs_datastream_t* instance = (nvs_datastream_t*)interface;
  const nvs_datastream_entry_t* entry = &instance->config->entries[key];

  char key_name[8];
  snprintf(key_name, sizeof(key_name), "k%u", (unsigned)key);
  flash_kv_write(instance->flash_kv, key_name, data, entry->size);

  datastream_on_change_args_t args = { .key = key, .data = data };
  event_publish(&interface->all_on_change, &args);
}

static void unsubscribe(i_datastream_t* interface, event_subscription_t* subscription)
{
  event_unsubscribe(&interface->all_on_change, subscription);
}

void nvs_datastream_init(nvs_datastream_t* instance,
                         const nvs_datastream_config_t* config,
                         i_flash_kv_t* flash_kv)
{
  instance->config   = config;
  instance->flash_kv = flash_kv;

  instance->interface = (i_datastream_t){
    .read        = read,
    .write       = write,
    .contains    = contains,
    .size        = size,
    .unsubscribe = unsubscribe,
  };

  event_init(&instance->interface.all_on_change);
}
