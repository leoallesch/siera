#include "siera/ds_composite.h"

static void republish(void* context, const void* args)
{
  siera_ds_composite_t* instance = (siera_ds_composite_t*)context;
  const siera_ds_on_change_args_t* change_args = (const siera_ds_on_change_args_t*)args;

  siera_ds_on_change_args_t composite_args = {
    .key = change_args->key,
    .data = change_args->data,
  };

  siera_event_publish(&instance->on_change, &composite_args);
}

static i_siera_ds_t* find_route(siera_ds_composite_t* instance, siera_dsk_t key)
{
  for(uint8_t i = 0; i < instance->count; i++) {
    if(siera_ds_contains(instance->routes[i].interface, key)) {
      return instance->routes[i].interface;
    }
  }

  return NULL;
}

static void read(i_siera_ds_t* interface, siera_dsk_t key, void* buf)
{
  siera_ds_composite_t* instance = (siera_ds_composite_t*)interface;
  i_siera_ds_t* route = find_route(instance, key);

  if(route) {
    siera_ds_read(route, key, buf);
  }
}

static void write(i_siera_ds_t* interface, siera_dsk_t key, const void* buf)
{
  siera_ds_composite_t* instance = (siera_ds_composite_t*)interface;
  i_siera_ds_t* route = find_route(instance, key);

  if(route) {
    siera_ds_write(route, key, buf);
  }
}

static bool contains(i_siera_ds_t* interface, siera_dsk_t key)
{
  siera_ds_composite_t* instance = (siera_ds_composite_t*)interface;

  for(uint8_t i = 0; i < instance->count; i++) {
    if(siera_ds_contains(instance->routes[i].interface, key)) {
      return true;
    }
  }

  return false;
}

static size_t size(i_siera_ds_t* interface, siera_dsk_t key)
{
  siera_ds_composite_t* instance = (siera_ds_composite_t*)interface;
  i_siera_ds_t* route = find_route(instance, key);

  if(route) {
    return siera_ds_size(route, key);
  }

  return 0;
}

static siera_event_t* on_change(i_siera_ds_t* interface)
{
  siera_ds_composite_t* instance = (siera_ds_composite_t*)interface;
  return &instance->on_change;
}

static const i_siera_ds_api_t _api = {
  .read = read,
  .write = write,
  .contains = contains,
  .size = size,
  .on_change = on_change,
};

void siera_ds_composite_init(
  siera_ds_composite_t* instance,
  siera_composite_route_t* routes,
  uint8_t count)
{
  instance->interface.api = &_api;
  instance->routes = routes;
  instance->count = count;

  siera_event_init(&instance->on_change);

  for(int i = 0; i < count; i++) {
    siera_event_t* event = siera_ds_on_change(routes[i].interface);
    siera_event_sub_init(&instance->routes[i].subscription, republish, instance);
    siera_event_subscribe(event, &instance->routes[i].subscription);
  }
}
