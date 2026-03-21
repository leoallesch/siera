#pragma once

#include "i_flash_kv.h"
#include "nvs_flash.h"

typedef struct {
  i_flash_kv_t interface;
  nvs_handle_t handle;
} esp32_flash_kv_t;

void esp32_flash_kv_init(esp32_flash_kv_t* instance, const char* namespace);
