#ifndef SIERA_HAL_ADC_H
#define SIERA_HAL_ADC_H

#include <stdint.h>

typedef uint16_t siera_adc_counts_t;

typedef struct siera_hal_adc_t {
    int (*read)(struct siera_hal_adc_t *self, uint32_t channel, siera_adc_counts_t *value);
} siera_hal_adc_t;

static inline int siera_adc_read(siera_hal_adc_t *self, uint32_t channel,
                                  siera_adc_counts_t *value)
{
    return self->read(self, channel, value);
}

#endif /* SIERA_HAL_ADC_H */
