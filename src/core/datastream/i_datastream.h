#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "event.h"

typedef uint16_t datastream_key_t;

typedef struct {
  datastream_key_t key;
  const void* data;
} datastream_on_change_args_t;

typedef struct i_datastream_t i_datastream_t;

typedef struct i_datastream_t {
  void (*read)(i_datastream_t* interface, datastream_key_t key, void* out);
  void (*write)(i_datastream_t* interface, datastream_key_t key, const void* data);
  bool (*contains)(i_datastream_t* interface, datastream_key_t key);
  uint8_t (*size)(i_datastream_t* interface, datastream_key_t key);
  void (*subscribe)(i_datastream_t* interface, datastream_key_t key, event_subscription_t* subscription);
  void (*subscribe_all)(i_datastream_t* interface, event_subscription_t* subscription);
  void (*unsubscribe)(i_datastream_t* interface, event_subscription_t* subscription);
} i_datastream_t;

static inline void datastream_read(i_datastream_t* interface, datastream_key_t key, void* out)
{
  interface->read(interface, key, out);
}

static inline void datastream_write(i_datastream_t* interface, datastream_key_t key, const void* data)
{
  interface->write(interface, key, data);
}

static inline bool datastream_contains(i_datastream_t* interface, datastream_key_t key)
{
  return interface->contains(interface, key);
}

static inline uint8_t datastream_size(i_datastream_t* interface, datastream_key_t key)
{
  return interface->size(interface, key);
}

static inline void datastream_subscribe(i_datastream_t* interface, datastream_key_t key, event_subscription_t* subscription)
{
  interface->subscribe(interface, key, subscription);
}

static inline void datastream_subscribe_all(i_datastream_t* interface, event_subscription_t* subscription)
{
  interface->subscribe_all(interface, subscription);
}

static inline void datastream_unsubscribe(i_datastream_t* interface, event_subscription_t* subscription)
{
  interface->unsubscribe(interface, subscription);
}
