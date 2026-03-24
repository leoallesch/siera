#ifndef SIERA_HAL_RTC_H
#define SIERA_HAL_RTC_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint16_t year;
  uint8_t  month;
  uint8_t  day;
  uint8_t  day_of_week;  // 0 = Monday ... 6 = Sunday
  uint8_t  hour;
  uint8_t  minute;
  uint8_t  second;
} siera_hal_rtc_datetime_t;

typedef struct siera_hal_rtc_t {
  bool (*get_datetime)(struct siera_hal_rtc_t* self, siera_hal_rtc_datetime_t* out);
  void (*set_datetime)(struct siera_hal_rtc_t* self, const siera_hal_rtc_datetime_t* datetime);
} siera_hal_rtc_t;

static inline bool siera_hal_rtc_get_datetime(siera_hal_rtc_t* self, siera_hal_rtc_datetime_t* out)
{
  return self->get_datetime(self, out);
}

static inline void siera_hal_rtc_set_datetime(siera_hal_rtc_t* self, const siera_hal_rtc_datetime_t* datetime)
{
  self->set_datetime(self, datetime);
}

#endif /* SIERA_HAL_RTC_H */
