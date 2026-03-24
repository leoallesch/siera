#ifndef SIERA_KEY_MANAGER_H
#define SIERA_KEY_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#include "siera/ds.h"
#include "siera/event.h"
#include "siera/key_event.h"
#include "siera/timer.h"

typedef struct {
  siera_ds_t*        ds;
  siera_timer_mgr_t* timers;

  siera_ds_key_t        output_key;
  const siera_ds_key_t* input_keys;
  uint8_t               key_count;

  siera_timer_t      timer;
  uint32_t           long_press_duration_ms;

  siera_event_sub_t  ds_sub;
  uint8_t            seq;

  siera_ds_key_t     current_key;
  bool               pressed;
  bool               long_pressed;
} siera_key_manager_t;

void siera_key_manager_init(
  siera_key_manager_t*  instance,
  siera_ds_t*           ds,
  siera_timer_mgr_t*    timers,
  siera_ds_key_t        output_key,
  uint32_t              long_press_duration_ms,
  const siera_ds_key_t* input_keys,
  uint8_t               input_keys_count);

#endif /* SIERA_KEY_MANAGER_H */
