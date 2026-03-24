#ifndef SIERA_DS_STREAM_H
#define SIERA_DS_STREAM_H

#include <stdint.h>
#include <stddef.h>

/* siera_ds_key_t is defined in ds_keys.h; included by ds.h after ds_stream.h */
#include "siera/ds_keys.h"

typedef struct {
    int (*read)(void *ctx, siera_ds_key_t key, void *buf, size_t size);
    int (*write)(void *ctx, siera_ds_key_t key, const void *buf, size_t size);
} siera_ds_stream_api_t;

typedef struct {
    const siera_ds_stream_api_t *api;
    void                        *ctx;
} siera_ds_stream_t;

typedef enum {
    SIERA_DS_RAM,
    SIERA_DS_NVS,
    SIERA_DS_HARDWARE_START, /* sentinel: all types >= this are hardware streams */
    SIERA_DS_GPIO = SIERA_DS_HARDWARE_START,
    SIERA_DS_ADC,
    SIERA_DS_PWM,
    SIERA_DS_I2C,
    SIERA_DS_SPI,
    SIERA_DS_STREAM_TYPE_COUNT
} siera_ds_stream_type_t;

#endif
