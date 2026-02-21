#pragma once

#include "event.h"
#include "i_datastream.h"

typedef struct {
  i_datastream_t interface;
  i_datastream_t** streams;
  uint8_t count;
} composite_datastream_t;

void composite_datastream_init(composite_datastream_t* instance, i_datastream_t** streams, uint8_t count);
