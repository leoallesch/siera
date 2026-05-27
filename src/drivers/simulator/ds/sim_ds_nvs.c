#include "siera/sim_ds_nvs.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static siera_sim_ds_nvs_t* self_of(i_siera_ds_t* interface)
{
  return (siera_sim_ds_nvs_t*)interface;
}

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

static int _read(i_siera_ds_t* interface, siera_dsk_t key, void* buf, size_t size)
{
  siera_sim_ds_nvs_t* self = self_of(interface);
  char key_name[8];
  snprintf(key_name, sizeof(key_name), "k%u", (unsigned)key);

  const siera_sim_ds_nvs_entry_t* entry = find_entry(self, key_name);
  if(!entry || entry->size != (uint8_t)size)
    return -1;
  memcpy(buf, entry->data, size);
  return 0;
}

static int _write(i_siera_ds_t* interface, siera_dsk_t key, const void* buf, size_t size)
{
  siera_sim_ds_nvs_t* self = self_of(interface);
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

  if(entry->size == (uint8_t)size && memcmp(entry->data, buf, size) == 0)
    return 0;

  memcpy(entry->data, buf, size);
  entry->size = (uint8_t)size;
  flush(self);

  siera_ds_on_change_args_t args = { .key = key, .data = buf };
  siera_event_publish(&self->on_change, &args);
  return 0;
}

static bool _contains(i_siera_ds_t* interface, siera_dsk_t key)
{
  siera_sim_ds_nvs_t* self = self_of(interface);
  char key_name[8];
  snprintf(key_name, sizeof(key_name), "k%u", (unsigned)key);
  return find_entry(self, key_name) != NULL;
}

static size_t _size(i_siera_ds_t* interface, siera_dsk_t key)
{
  siera_sim_ds_nvs_t* self = self_of(interface);
  char key_name[8];
  snprintf(key_name, sizeof(key_name), "k%u", (unsigned)key);
  const siera_sim_ds_nvs_entry_t* entry = find_entry(self, key_name);
  return entry ? entry->size : 0;
}

static siera_event_t* _on_change(i_siera_ds_t* interface)
{
  return &self_of(interface)->on_change;
}

static const i_siera_ds_api_t _api = {
  .read = _read,
  .write = _write,
  .contains = _contains,
  .size = _size,
  .on_change = _on_change,
};

void siera_sim_ds_nvs_init(siera_sim_ds_nvs_t* self, const char* filepath)
{
  self->interface.api = &_api;
  self->filepath = filepath;
  self->count = 0;
  memset(self->entries, 0, sizeof(self->entries));
  siera_event_init(&self->on_change);

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
