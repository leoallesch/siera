#ifndef SIERA_ESP32_ADC_H
#define SIERA_ESP32_ADC_H

#include "siera/hal_adc.h"

#include "esp_adc/adc_oneshot.h"

siera_hal_adc_t *siera_esp32_adc_init(adc_unit_t unit);

#endif /* SIERA_ESP32_ADC_H */
