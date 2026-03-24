#ifndef SIERA_SIM_RTC_H
#define SIERA_SIM_RTC_H

#include "siera/hal_rtc.h"

// Returns a pointer to a simulator RTC instance initialised to the host
// system's current local time.  Subsequent set_datetime calls update the
// internal offset so that get_datetime continues to advance from the set point.
siera_hal_rtc_t* siera_sim_rtc_init(void);

#endif /* SIERA_SIM_RTC_H */
