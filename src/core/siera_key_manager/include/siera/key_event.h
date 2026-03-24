#ifndef SIERA_KEY_EVENT_H
#define SIERA_KEY_EVENT_H

#include <stdint.h>

typedef enum {
  SIERA_KEY_EVENT_NONE = 0,
  SIERA_KEY_EVENT_PRESS,
  SIERA_KEY_EVENT_RELEASE,
  SIERA_KEY_EVENT_LONGPRESS,
  SIERA_KEY_EVENT_PRESSANDRELEASE,
  SIERA_KEY_EVENT_LONGPRESSANDRELEASE,
} siera_key_event_t;

typedef struct {
  uint16_t key;
  siera_key_event_t event;
  uint8_t seq;
} siera_key_event_data_t;

#endif /* SIERA_KEY_EVENT_H */
