#include "siera/sim_ds_nvs.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static siera_sim_ds_nvs_entry_t* find_entry(siera_sim_ds_nvs_t* self, const char* key)
{
  for(uint8_t i = 0; i < self->count; i++) {
    if(strncmp(self->entries[i].key, key, SIERA_SIM_DS_NVS_MAX_KEY_LEN) == 0)
      return &self->entries[i];
  }
  return NULL;
}

static void flush(siera_sim_ds_nvs_t* self)
{
  FILE* f = fopen(self->filepath, "wb");
  if(!f)
    return;
  fwrite(&self->count, sizeof(self->count), 1, f);
  fwrite(self->entries, sizeof(siera_sim_ds_nvs_entry_t), self->count, f);
  fclose(f);
}

static int _read(void* ctx, siera_ds_key_t key, void* buf, size_t size)
{
  siera_sim_ds_nvs_t* self = (siera_sim_ds_nvs_t*)ctx;
  char key_name[8];
  snprintf(key_name, sizeof(key_name), "k%u", (unsigned)key);

  const siera_sim_ds_nvs_entry_t* entry = find_entry(self, key_name);
  if(!entry || entry->size != (uint8_t)size)
    return -1;
  memcpy(buf, entry->data, size);
  return 0;
}

static int _write(void* ctx, siera_ds_key_t key, const void* buf, size_t size)
{
  siera_sim_ds_nvs_t* self = (siera_sim_ds_nvs_t*)ctx;
  if(size > SIERA_SIM_DS_NVS_MAX_VALUE_SIZE)
    return -1;

  char key_name[8];
  snprintf(key_name, sizeof(key_name), "k%u", (unsigned)key);

  siera_sim_ds_nvs_entry_t* entry = find_entry(self, key_name);
  if(!entry) {
    if(self->count >= SIERA_SIM_DS_NVS_MAX_ENTRIES)
      return -1;
    entry = &self->entries[self->count++];
    strncpy(entry->key, key_name, SIERA_SIM_DS_NVS_MAX_KEY_LEN);
    entry->key[SIERA_SIM_DS_NVS_MAX_KEY_LEN] = '\0';
  }

  memcpy(entry->data, buf, size);
  entry->size = (uint8_t)size;
  flush(self);
  return 0;
}

static const siera_ds_stream_api_t _api = { _read, _write };

void siera_sim_ds_nvs_init(siera_sim_ds_nvs_t* self, const char* filepath)
{
  self->filepath = filepath;
  self->count = 0;
  memset(self->entries, 0, sizeof(self->entries));
  self->stream.api = &_api;
  self->stream.ctx = self;

  FILE* f = fopen(filepath, "rb");
  if(!f)
    return;
  fread(&self->count, sizeof(self->count), 1, f);
  if(self->count > SIERA_SIM_DS_NVS_MAX_ENTRIES)
    self->count = 0;
  else
    fread(self->entries, sizeof(siera_sim_ds_nvs_entry_t), self->count, f);
  fclose(f);
}
