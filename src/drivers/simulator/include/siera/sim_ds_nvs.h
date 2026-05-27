#ifndef SIERA_SIM_DS_NVS_H
#define SIERA_SIM_DS_NVS_H

#include "siera/ds.h"

/*
 * File-backed NVS datastream for the host simulator.
 *
 * Implements i_siera_ds_t with variable-size blobs persisted to disk.
 *
 * Usage:
 *   static siera_sim_ds_nvs_t nvs;
 *   siera_sim_ds_nvs_init(&nvs, "siera_nvs.bin");
 *   siera_sim_ds_nvs_register(&nvs, DSK_BRIGHTNESS, sizeof(uint8_t));
 *   siera_sim_ds_nvs_register(&nvs, DSK_ALARM_HOUR, sizeof(uint8_t));
 *   ...
 *   siera_ds_write(&nvs.interface, DSK_BRIGHTNESS, &val);
 */

typedef struct {
  siera_dsk_t key;
  uint16_t size;
  void* data; /* heap, length = size */
} siera_sim_ds_nvs_entry_t;

typedef struct {
  i_siera_ds_t interface;
  siera_event_t on_change;
  const char* filepath;
  siera_sim_ds_nvs_entry_t* entries; /* heap, length = count */
  uint16_t count;
  uint16_t capacity;
} siera_sim_ds_nvs_t;

void siera_sim_ds_nvs_init(siera_sim_ds_nvs_t* self, const char* filepath);
void siera_sim_ds_nvs_deinit(siera_sim_ds_nvs_t* self);

/* Declare a key with its blob size. Required before first write of a new key.
   Idempotent: re-registering the same key with the same size is a no-op;
   re-registering with a different size resets the blob. */
void siera_sim_ds_nvs_register(siera_sim_ds_nvs_t* self, siera_dsk_t key, uint16_t size);

#endif /* SIERA_SIM_DS_NVS_H */
