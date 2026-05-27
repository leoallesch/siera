#ifndef DS_RAM_H
#define DS_RAM_H

#include "siera/ds.h"
#include "siera/event.h"

typedef struct {
  uint16_t offset;
  uint8_t size;
} siera_ram_config_t;

typedef struct {
  i_siera_ds_t interface;
  const siera_ram_config_t* configs;
  uint16_t count;
  uint8_t* storage;
  siera_event_t on_change;
} siera_ds_ram_t;

void siera_ds_ram_init(
  siera_ds_ram_t* instance,
  const siera_ram_config_t* configs,
  uint16_t count,
  void* storage);

#endif
