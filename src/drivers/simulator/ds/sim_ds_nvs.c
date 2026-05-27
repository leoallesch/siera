#include "siera/sim_ds_nvs.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static siera_sim_ds_nvs_t* self_of(i_siera_ds_t* interface)
{
  return (siera_sim_ds_nvs_t*)interface;
}

static siera_sim_ds_nvs_entry_t* find_entry(siera_sim_ds_nvs_t* self, siera_dsk_t key)
{
  for(uint16_t i = 0; i < self->count; i++) {
    if(self->entries[i].key == key)
      return &self->entries[i];
  }
  return NULL;
}

static siera_sim_ds_nvs_entry_t* append_entry(siera_sim_ds_nvs_t* self)
{
  if(self->count == self->capacity) {
    uint16_t new_cap = self->capacity ? self->capacity * 2 : 8;
    self->entries = realloc(self->entries, new_cap * sizeof(*self->entries));
    self->capacity = new_cap;
  }
  siera_sim_ds_nvs_entry_t* e = &self->entries[self->count++];
  e->key = 0;
  e->size = 0;
  e->data = NULL;
  return e;
}

static void flush(siera_sim_ds_nvs_t* self)
{
  FILE* f = fopen(self->filepath, "wb");
  if(!f)
    return;
  fwrite(&self->count, sizeof(self->count), 1, f);
  for(uint16_t i = 0; i < self->count; i++) {
    siera_sim_ds_nvs_entry_t* e = &self->entries[i];
    fwrite(&e->key, sizeof(e->key), 1, f);
    fwrite(&e->size, sizeof(e->size), 1, f);
    if(e->size)
      fwrite(e->data, 1, e->size, f);
  }
  fclose(f);
}

static void load(siera_sim_ds_nvs_t* self)
{
  FILE* f = fopen(self->filepath, "rb");
  if(!f)
    return;

  uint16_t count = 0;
  if(fread(&count, sizeof(count), 1, f) != 1) {
    fclose(f);
    return;
  }

  for(uint16_t i = 0; i < count; i++) {
    siera_dsk_t key;
    uint16_t size;
    if(fread(&key, sizeof(key), 1, f) != 1)
      break;
    if(fread(&size, sizeof(size), 1, f) != 1)
      break;

    siera_sim_ds_nvs_entry_t* e = append_entry(self);
    e->key = key;
    e->size = size;
    e->data = size ? malloc(size) : NULL;
    if(size && fread(e->data, 1, size, f) != size)
      break;
  }
  fclose(f);
}

/* -------------------------------------------------------------------------
 * i_siera_ds_t implementation
 * -------------------------------------------------------------------------*/

static void _read(i_siera_ds_t* interface, siera_dsk_t key, void* buf)
{
  siera_sim_ds_nvs_t* self = self_of(interface);
  const siera_sim_ds_nvs_entry_t* e = find_entry(self, key);
  if(!e || !e->size)
    return;
  memcpy(buf, e->data, e->size);
}

static void _write(i_siera_ds_t* interface, siera_dsk_t key, const void* buf)
{
  siera_sim_ds_nvs_t* self = self_of(interface);
  siera_sim_ds_nvs_entry_t* e = find_entry(self, key);
  if(!e || !e->size)
    return; /* key not registered — caller must register with known size first */

  if(memcmp(e->data, buf, e->size) == 0)
    return;

  memcpy(e->data, buf, e->size);

  siera_ds_on_change_args_t args = { .key = key, .data = buf };
  siera_event_publish(&self->on_change, &args);

  flush(self);
}

static bool _contains(i_siera_ds_t* interface, siera_dsk_t key)
{
  return find_entry(self_of(interface), key) != NULL;
}

static size_t _size(i_siera_ds_t* interface, siera_dsk_t key)
{
  const siera_sim_ds_nvs_entry_t* e = find_entry(self_of(interface), key);
  return e ? e->size : 0;
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

/* -------------------------------------------------------------------------
 * Public
 * -------------------------------------------------------------------------*/

void siera_sim_ds_nvs_init(siera_sim_ds_nvs_t* self, const char* filepath)
{
  self->interface.api = &_api;
  self->filepath = filepath;
  self->entries = NULL;
  self->count = 0;
  self->capacity = 0;
  siera_event_init(&self->on_change);

  load(self);
}

void siera_sim_ds_nvs_deinit(siera_sim_ds_nvs_t* self)
{
  for(uint16_t i = 0; i < self->count; i++)
    free(self->entries[i].data);
  free(self->entries);
  self->entries = NULL;
  self->count = 0;
  self->capacity = 0;
}

void siera_sim_ds_nvs_register(siera_sim_ds_nvs_t* self, siera_dsk_t key, uint16_t size)
{
  siera_sim_ds_nvs_entry_t* e = find_entry(self, key);
  if(e) {
    if(e->size == size)
      return; /* same registration */
    free(e->data);
    e->size = size;
    e->data = size ? calloc(1, size) : NULL;
    flush(self);
    return;
  }

  e = append_entry(self);
  e->key = key;
  e->size = size;
  e->data = size ? calloc(1, size) : NULL;
  flush(self);
}
