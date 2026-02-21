#include "composite_datastream.h"

static i_datastream_t* find_stream(composite_datastream_t* instance, datastream_key_t key)
{
  for(uint16_t i = 0; i < instance->count; i++) {
    if(datastream_contains(instance->streams[i], key)) {
      return instance->streams[i];
    }
  }
  return NULL; // Key not found; handle error as needed.
}

static void read(i_datastream_t* interface, datastream_key_t key, void* out)
{
  composite_datastream_t* instance = (composite_datastream_t*)interface;
  i_datastream_t* stream = find_stream(instance, key);
  if(stream) {
    datastream_read(stream, key, out);
  }
}

static void write(i_datastream_t* interface, datastream_key_t key, const void* data)
{
  composite_datastream_t* instance = (composite_datastream_t*)interface;
  i_datastream_t* stream = find_stream(instance, key);
  if(stream) {
    datastream_write(stream, key, data);
  }
}

static bool contains(i_datastream_t* interface, datastream_key_t key)
{
  composite_datastream_t* instance = (composite_datastream_t*)interface;
  return find_stream(instance, key) != NULL;
}

static uint8_t size(i_datastream_t* interface, datastream_key_t key)
{
  composite_datastream_t* instance = (composite_datastream_t*)interface;
  i_datastream_t* stream = find_stream(instance, key);
  return stream ? datastream_size(stream, key) : 0;
}

static void subscribe(i_datastream_t* interface, datastream_key_t key, event_subscription_t* subscription)
{
  composite_datastream_t* instance = (composite_datastream_t*)interface;
  i_datastream_t* stream = find_stream(instance, key);
  if(stream) {
    datastream_subscribe(stream, key, subscription);
  }
}

static void subscribe_all(i_datastream_t* interface, event_subscription_t* subscription)
{
  composite_datastream_t* instance = (composite_datastream_t*)interface;
  for(uint16_t i = 0; i < instance->count; i++) {
    datastream_subscribe_all(instance->streams[i], subscription);
  }
}

static void unsubscribe(i_datastream_t* interface, event_subscription_t* subscription)
{
  composite_datastream_t* instance = (composite_datastream_t*)interface;
  for(uint16_t i = 0; i < instance->count; i++) {
    datastream_unsubscribe(instance->streams[i], subscription);
  }
}

void composite_datastream_init(composite_datastream_t* instance, i_datastream_t** streams, uint8_t count)
{
  instance->streams = streams;
  instance->count = count;

  instance->interface = (i_datastream_t){
    .read = read,
    .write = write,
    .contains = contains,
    .size = size,
    .subscribe = subscribe,
    .subscribe_all = subscribe_all,
    .unsubscribe = unsubscribe,
  };
}
