#include <stdio.h>
#include <string.h>

#include "simulator_flash_kv.h"

static simulator_flash_kv_entry_t* find_entry(simulator_flash_kv_t* instance, const char* key)
{
  for(uint8_t i = 0; i < instance->count; i++) {
    if(strncmp(instance->entries[i].key, key, SIMULATOR_FLASH_KV_MAX_KEY_LEN) == 0)
      return &instance->entries[i];
  }
  return NULL;
}

static void flush(simulator_flash_kv_t* instance)
{
  FILE* f = fopen(instance->filepath, "wb");
  if(!f)
    return;
  fwrite(&instance->count, sizeof(instance->count), 1, f);
  fwrite(instance->entries, sizeof(simulator_flash_kv_entry_t), instance->count, f);
  fclose(f);
}

static void kv_write(i_flash_kv_t* self, const char* key, const void* data, uint8_t size)
{
  simulator_flash_kv_t* instance = (simulator_flash_kv_t*)self;
  if(size > SIMULATOR_FLASH_KV_MAX_VALUE_SIZE)
    return;

  simulator_flash_kv_entry_t* entry = find_entry(instance, key);
  if(!entry) {
    if(instance->count >= SIMULATOR_FLASH_KV_MAX_ENTRIES)
      return;
    entry = &instance->entries[instance->count++];
    strncpy(entry->key, key, SIMULATOR_FLASH_KV_MAX_KEY_LEN);
    entry->key[SIMULATOR_FLASH_KV_MAX_KEY_LEN] = '\0';
  }

  memcpy(entry->data, data, size);
  entry->size = size;
  flush(instance);
}

static bool kv_read(i_flash_kv_t* self, const char* key, void* out, uint8_t size)
{
  simulator_flash_kv_t* instance = (simulator_flash_kv_t*)self;
  const simulator_flash_kv_entry_t* entry = find_entry(instance, key);
  if(!entry || entry->size != size)
    return false;
  memcpy(out, entry->data, size);
  return true;
}

static void kv_erase(i_flash_kv_t* self, const char* key)
{
  simulator_flash_kv_t* instance = (simulator_flash_kv_t*)self;
  for(uint8_t i = 0; i < instance->count; i++) {
    if(strncmp(instance->entries[i].key, key, SIMULATOR_FLASH_KV_MAX_KEY_LEN) == 0) {
      instance->entries[i] = instance->entries[--instance->count];
      flush(instance);
      return;
    }
  }
}

void simulator_flash_kv_init(simulator_flash_kv_t* instance, const char* filepath)
{
  instance->filepath = filepath;
  instance->count    = 0;

  instance->interface = (i_flash_kv_t){
    .write = kv_write,
    .read  = kv_read,
    .erase = kv_erase,
  };

  FILE* f = fopen(filepath, "rb");
  if(!f)
    return;
  fread(&instance->count, sizeof(instance->count), 1, f);
  if(instance->count > SIMULATOR_FLASH_KV_MAX_ENTRIES)
    instance->count = 0;
  else
    fread(instance->entries, sizeof(simulator_flash_kv_entry_t), instance->count, f);
  fclose(f);
}
