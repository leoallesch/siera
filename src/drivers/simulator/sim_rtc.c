#include "siera/sim_rtc.h"

#include <stdint.h>
#include <time.h>

// Offset in seconds applied on top of the host clock after set_datetime.
static int64_t s_offset_seconds = 0;

static bool _get_datetime(siera_hal_rtc_t* self, siera_hal_rtc_datetime_t* out)
{
  (void)self;
  time_t now = time(NULL) + (time_t)s_offset_seconds;
  struct tm t;
  localtime_r(&now, &t);

  out->year        = (uint16_t)(t.tm_year + 1900);
  out->month       = (uint8_t)(t.tm_mon + 1);
  out->day         = (uint8_t)t.tm_mday;
  out->day_of_week = (uint8_t)((t.tm_wday + 6) % 7);  // tm: 0=Sun → 0=Mon
  out->hour        = (uint8_t)t.tm_hour;
  out->minute      = (uint8_t)t.tm_min;
  out->second      = (uint8_t)t.tm_sec;
  return true;
}

static void _set_datetime(siera_hal_rtc_t* self, const siera_hal_rtc_datetime_t* datetime)
{
  (void)self;
  struct tm t = {
    .tm_year  = datetime->year - 1900,
    .tm_mon   = datetime->month - 1,
    .tm_mday  = datetime->day,
    .tm_hour  = datetime->hour,
    .tm_min   = datetime->minute,
    .tm_sec   = datetime->second,
    .tm_isdst = -1,
  };
  time_t target = mktime(&t);
  time_t now    = time(NULL);
  s_offset_seconds = (int64_t)(target - now);
}

siera_hal_rtc_t* siera_sim_rtc_init(void)
{
  static siera_hal_rtc_t rtc = {
    .get_datetime = _get_datetime,
    .set_datetime = _set_datetime,
  };
  return &rtc;
}
