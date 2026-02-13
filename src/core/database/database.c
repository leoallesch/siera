#include "database.h"

void database_read(s_database_t* instance, datastream_key_t key, void* out)
{
  for(uint8_t i = 0; i < instance->count; i++) {
    if(datastream_contains(instance->streams[i], key)) {
      datastream_read(instance->streams[i], key, out);
      return;
    }
  }
}

void database_write(s_database_t* instance, datastream_key_t key, const void* data)
{
  for(uint8_t i = 0; i < instance->count; i++) {
    if(datastream_contains(instance->streams[i], key)) {
      datastream_write(instance->streams[i], key, data);
      return;
    }
  }
}

bool database_contains(s_database_t* instance, datastream_key_t key)
{
  for(uint8_t i = 0; i < instance->count; i++) {
    if(datastream_contains(instance->streams[i], key)) {
      return true;
    }
  }
  return false;
}

uint8_t database_size(s_database_t* instance, datastream_key_t key)
{
  for(uint8_t i = 0; i < instance->count; i++) {
    if(datastream_contains(instance->streams[i], key)) {
      return datastream_size(instance->streams[i], key);
    }
  }
  return 0;
}

void database_init(s_database_t* instance, i_datastream_t** streams, uint8_t count)
{
  instance->streams = streams;
  instance->count = count;
}
