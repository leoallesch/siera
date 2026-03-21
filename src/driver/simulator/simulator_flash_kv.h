#pragma once

#include "i_flash_kv.h"

// simulator_flash_kv_t: file-backed implementation of i_flash_kv_t for the host simulator.
// Persists key-value blobs to a binary file; loaded on init, flushed on every write/erase.

#define SIMULATOR_FLASH_KV_MAX_ENTRIES 64
#define SIMULATOR_FLASH_KV_MAX_KEY_LEN 15
#define SIMULATOR_FLASH_KV_MAX_VALUE_SIZE 64

typedef struct {
  char    key[SIMULATOR_FLASH_KV_MAX_KEY_LEN + 1];
  uint8_t data[SIMULATOR_FLASH_KV_MAX_VALUE_SIZE];
  uint8_t size;
} simulator_flash_kv_entry_t;

typedef struct {
  i_flash_kv_t              interface;
  const char*               filepath;
  simulator_flash_kv_entry_t entries[SIMULATOR_FLASH_KV_MAX_ENTRIES];
  uint8_t                   count;
} simulator_flash_kv_t;

void simulator_flash_kv_init(simulator_flash_kv_t* instance, const char* filepath);
