#ifndef SIERA_ESP32_DS_ADC_H
#define SIERA_ESP32_DS_ADC_H

#include "siera/ds_stream.h"
#include "siera/hal_adc.h"

/*
 * ADC datastream stream for siera_ds.
 *
 * Maps siera_ds keys typed SIERA_DS_ADC to ADC channels via a lookup table.
 * Reads are forwarded to the HAL ADC interface and the raw value is stored in
 * the key's cache slot.  ADC keys are read-only by convention; write calls
 * return -1.
 *
 * Usage:
 *   static siera_esp32_ds_adc_channel_t channels[] = {
 *     { SIERA_DS_KEY_BATTERY_MV, ADC_CHANNEL_6 },
 *   };
 *   static siera_esp32_ds_adc_t adc_stream;
 *   siera_esp32_ds_adc_init(&adc_stream, &adc_hal.interface, channels, 1);
 *
 *   // Bind to datastream:
 *   siera_ds_stream_binding_t bindings[] = {
 *     { SIERA_DS_ADC, &adc_stream.stream },
 *   };
 */

typedef struct {
  siera_ds_key_t key;
  uint32_t       channel;
} siera_esp32_ds_adc_channel_t;

typedef struct {
  siera_ds_stream_t                   stream;
  siera_hal_adc_t                    *hal;
  const siera_esp32_ds_adc_channel_t *channels;
  size_t                              channel_count;
} siera_esp32_ds_adc_t;

void siera_esp32_ds_adc_init(siera_esp32_ds_adc_t               *self,
                              siera_hal_adc_t                    *hal,
                              const siera_esp32_ds_adc_channel_t *channels,
                              size_t                              channel_count);

#endif /* SIERA_ESP32_DS_ADC_H */
