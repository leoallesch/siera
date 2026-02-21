#include <string.h>
#include "i_datastream.h"
#include "ram_datastream.h"

static uint16_t offset(ram_datastream_t* instance, datastream_key_t key)
{
  return instance->config->entries[key].offset;
}

static bool contains(i_datastream_t* interface, datastream_key_t key)
{
  ram_datastream_t* instance = (ram_datastream_t*)interface;
  return key < instance->config->count && instance->config->entries[key].size > 0;
}

static uint8_t size(i_datastream_t* interface, datastream_key_t key)
{
  if(contains(interface, key)) {
    ram_datastream_t* instance = (ram_datastream_t*)interface;
    return instance->config->entries[key].size;
  }
  return 0;
}

static void read(i_datastream_t* interface, datastream_key_t key, void* out)
{
  if(contains(interface, key)) {
    ram_datastream_t* instance = (ram_datastream_t*)interface;
    const ram_datastream_entry_t* entry = &instance->config->entries[key];
    uint8_t* src = (uint8_t*)instance->storage + entry->offset;
    memcpy(out, src, entry->size);
  }
}

static void write(i_datastream_t* interface, datastream_key_t key, const void* data)
{
  if(contains(interface, key)) {
    ram_datastream_t* instance = (ram_datastream_t*)interface;
    uint8_t s = size(interface, key);
    void* location = (uint8_t*)instance->storage + instance->config->entries[key].offset;
    if(memcmp(location, data, s)) {
      memcpy(location, data, s);
      datastream_on_change_args_t args = {
        .key = key,
        .data = data,
      };

      ram_datastream_entry_t entry = instance->config->entries[key];
      event_publish(&entry.entry_on_change, &args);
      event_publish(&instance->all_on_change, &args);
    }
  }
}

void subscribe(i_datastream_t* interface, datastream_key_t key, event_subscription_t* subscription)
{
  if(contains(interface, key)) {
    ram_datastream_t* instance = (ram_datastream_t*)interface;
    event_subscribe(&instance->config->entries[key].entry_on_change, subscription);
  }
}

void subscribe_all(i_datastream_t* interface, event_subscription_t* subscription)
{
  ram_datastream_t* instance = (ram_datastream_t*)interface;
  event_subscribe(&instance->all_on_change, subscription);
}

void unsubscribe(i_datastream_t* interface, event_subscription_t* subscription)
{
  ram_datastream_t* instance = (ram_datastream_t*)interface;
  for(uint16_t i = 0; i < instance->config->count; i++) {
    event_unsubscribe(&instance->config->entries[i].entry_on_change, subscription);
  }
  event_unsubscribe(&instance->all_on_change, subscription);
}

void ram_datastream_init(ram_datastream_t* instance, const ram_datastream_config_t* config, void* storage)
{
  instance->config = config;
  instance->storage = storage;

  instance->interface = (i_datastream_t){
    .read = read,
    .write = write,
    .contains = contains,
    .size = size,
    .subscribe = subscribe,
    .subscribe_all = subscribe_all,
    .unsubscribe = unsubscribe,
  };

  datastream_key_t last_key = (datastream_key_t)(instance->config->count - 1);
  uint16_t last_offset = offset(instance, last_key);
  uint8_t last_size = size(&instance->interface, last_key);
  memset(instance->storage, 0, last_offset + last_size);

  for(uint16_t i = 0; i < config->count; i++) {
    event_init(&instance->config->entries[i].entry_on_change);
  }

  event_init(&instance->all_on_change);
}
