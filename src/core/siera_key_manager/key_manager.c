#include "siera/key_manager.h"

static void emit_event(siera_key_manager_t* instance, siera_dsk_t key, siera_key_event_t event)
{
  siera_key_event_data_t data = {
    .key = key,
    .event = event,
    .seq = instance->seq++,
  };
  siera_ds_write(instance->ds, instance->output_key, &data);
}

static void on_long_press_timeout(void* ctx)
{
  siera_key_manager_t* instance = (siera_key_manager_t*)ctx;
  instance->long_pressed = true;
  emit_event(instance, instance->current_key, SIERA_KEY_EVENT_LONGPRESS);
}

static void process_key(siera_key_manager_t* instance, siera_dsk_t key, bool pressed)
{
  instance->current_key = key;

  if(pressed && !instance->pressed) {
    instance->pressed = true;
    siera_timer_start(
      instance->timers,
      &instance->timer,
      on_long_press_timeout,
      instance,
      instance->long_press_duration_ms,
      false);
    emit_event(instance, key, SIERA_KEY_EVENT_PRESS);
  }
  else if(!pressed && instance->pressed) {
    instance->pressed = false;
    bool was_long_press = instance->long_pressed;
    instance->long_pressed = false;
    if(siera_timer_is_active(&instance->timer))
      siera_timer_stop(instance->timers, &instance->timer);
    emit_event(instance, key, SIERA_KEY_EVENT_RELEASE);
    emit_event(instance, key, was_long_press ? SIERA_KEY_EVENT_LONGPRESSANDRELEASE : SIERA_KEY_EVENT_PRESSANDRELEASE);
  }
}

static void on_ds_change(void* ctx, const void* args)
{
  siera_key_manager_t* instance = (siera_key_manager_t*)ctx;
  const siera_ds_on_change_args_t* change = (const siera_ds_on_change_args_t*)args;

  for(uint8_t i = 0; i < instance->key_count; i++) {
    if(change->key == instance->input_keys[i])
      process_key(instance, change->key, *(const bool*)change->data);
  }
}

void siera_key_manager_init(
  siera_key_manager_t* instance,
  i_siera_ds_t* ds,
  siera_timer_mgr_t* timers,
  siera_dsk_t output_key,
  uint32_t long_press_duration_ms,
  const siera_dsk_t* input_keys,
  uint8_t input_keys_count)
{
  instance->ds = ds;
  instance->timers = timers;
  instance->output_key = output_key;
  instance->long_press_duration_ms = long_press_duration_ms;
  instance->seq = 0;
  instance->input_keys = input_keys;
  instance->key_count = input_keys_count;
  instance->pressed = false;
  instance->long_pressed = false;

  siera_event_sub_init(&instance->ds_sub, on_ds_change, instance);
  siera_ds_subscribe_all(ds, &instance->ds_sub);
}
