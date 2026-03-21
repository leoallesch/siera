#pragma once

#include "i_datastream.h"
#include "i_flash_kv.h"

typedef struct {
  uint16_t offset;
  uint8_t  size;
} nvs_datastream_entry_t;

typedef struct {
  nvs_datastream_entry_t* entries;
  uint16_t                count;
} nvs_datastream_config_t;

typedef struct {
  i_datastream_t                 interface;
  const nvs_datastream_config_t* config;
  i_flash_kv_t*                  flash_kv;
} nvs_datastream_t;

void nvs_datastream_init(nvs_datastream_t* instance,
                         const nvs_datastream_config_t* config,
                         i_flash_kv_t* flash_kv);
