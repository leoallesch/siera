#pragma once

#include "i_datastream.h"
#include "event.h"

typedef struct {
  i_datastream_t** streams;
  uint8_t count;
} s_database_t;

void database_init(s_database_t* instance, i_datastream_t** streams, uint8_t count);

void database_read(s_database_t* instance, datastream_key_t key, void* out);
void database_write(s_database_t* instance, datastream_key_t key, const void* data);
bool database_contains(s_database_t* instance, datastream_key_t key);
uint8_t database_size(s_database_t* instance, datastream_key_t key);