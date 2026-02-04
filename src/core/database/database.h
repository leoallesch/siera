#pragma once

#include <stdint.h>

#include "event.h"

typedef uint16_t s_key_t;

typedef struct
{
  s_key_t key;
  const void* data;
} s_database_on_change_args_t;

typedef struct
{
  uint16_t offset;
  uint8_t size;
} s_database_entry_t;

typedef struct
{
  const s_database_entry_t* entries;
  uint16_t count;
} s_database_config_t;

typedef struct
{
  const s_database_config_t* config;
  void* storage;
  event_t on_change;
} s_database_t;

void database_init(s_database_t* database, const s_database_config_t* config, void* storage);

void database_read(s_database_t* database, s_key_t key, void* out);
void database_write(s_database_t* database, s_key_t key, const void* data);
bool database_contains(s_database_t* database, s_key_t key);
uint8_t database_size(s_database_t* database, s_key_t key);

void database_subscribe_all(s_database_t* database, event_subscription_t* subscription);