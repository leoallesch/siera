#pragma once

#include "event.h"
#include "i_datastream.h"

typedef struct
{
  uint16_t offset;
  uint8_t size;
  event_t entry_on_change;
} ram_datastream_entry_t;

typedef struct
{
  ram_datastream_entry_t* entries;
  uint16_t count;
} ram_datastream_config_t;

typedef struct
{
  i_datastream_t interface;
  const ram_datastream_config_t* config;
  void* storage;
  event_t all_on_change;
} ram_datastream_t;

void ram_datastream_init(ram_datastream_t* instance, const ram_datastream_config_t* config, void* storage);
