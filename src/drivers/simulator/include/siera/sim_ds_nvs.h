#ifndef SIERA_SIM_DS_NVS_H
#define SIERA_SIM_DS_NVS_H

#include "siera/ds_stream.h"

/*
 * NVS datastream stream for siera_ds on the host simulator.
 *
 * Implements SIERA_DS_NVS via a file-backed key-value store.
 * Keys are stored as "k<index>" strings; values are raw blobs.
 * The backing file is loaded on init and flushed on every write.
 *
 * Usage:
 *   static siera_sim_ds_nvs_t nvs_stream;
 *   siera_sim_ds_nvs_init(&nvs_stream, "siera_nvs.bin");
 *
 *   siera_ds_stream_binding_t bindings[] = {
 *     { SIERA_DS_NVS, &nvs_stream.stream },
 *   };
 */

#define SIERA_SIM_DS_NVS_MAX_ENTRIES    64
#define SIERA_SIM_DS_NVS_MAX_KEY_LEN    15
#define SIERA_SIM_DS_NVS_MAX_VALUE_SIZE 128

typedef struct {
  char    key[SIERA_SIM_DS_NVS_MAX_KEY_LEN + 1];
  uint8_t data[SIERA_SIM_DS_NVS_MAX_VALUE_SIZE];
  uint8_t size;
} siera_sim_ds_nvs_entry_t;

typedef struct {
  siera_ds_stream_t         stream;
  const char*               filepath;
  siera_sim_ds_nvs_entry_t  entries[SIERA_SIM_DS_NVS_MAX_ENTRIES];
  uint8_t                   count;
} siera_sim_ds_nvs_t;

void siera_sim_ds_nvs_init(siera_sim_ds_nvs_t* self, const char* filepath);

#endif /* SIERA_SIM_DS_NVS_H */
