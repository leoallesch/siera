#include <string.h>
#include "database.h"

void database_init(s_database_t* database, const s_database_config_t* config, void* storage)
{
  database->config = config;
  database->storage = storage;

  uint8_t last_key = config->count - 1;
  const s_database_entry_t* last_entry = &config->entries[last_key];
  memset(storage, 0, last_entry->offset + last_entry->size);

  event_init(&database->on_change);
}

void database_read(s_database_t* database, s_key_t key, void* out)
{
  const s_database_entry_t* entry = &database->config->entries[key];
  uint8_t* src = (uint8_t*)database->storage + entry->offset;
  memcpy(out, src, entry->size);
}

void database_write(s_database_t* database, s_key_t key, const void* data)
{
  uint8_t size = database_size(database, key);
  void* location = (uint8_t*)database->storage + database->config->entries[key].offset;
  if(memcmp(location, data, size)) {
    memcpy(location, data, size);
    s_database_on_change_args_t args = {
      .key = key,
      .data = data,
    };
    event_publish(&database->on_change, &args);
  }
}

bool database_contains(s_database_t* database, s_key_t key)
{
  return key < database->config->count;
}

uint8_t database_size(s_database_t* database, s_key_t key)
{
  return database->config->entries[key].size;
}

void database_subscribe_all(s_database_t* database, event_subscription_t* subscription)
{
  event_subscribe(&database->on_change, subscription);
}
