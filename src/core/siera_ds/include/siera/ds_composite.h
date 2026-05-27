#ifndef DS_COMPOSITE_H
#define DS_COMPOSITE_H

#include <stdint.h>
#include "siera/ds.h"

typedef struct {
  i_siera_ds_t* interface;
  siera_event_sub_t subscription;
} siera_composite_route_t;

typedef struct {
  i_siera_ds_t interface;
  siera_composite_route_t* routes;
  uint8_t count;
  siera_event_t on_change;
} siera_ds_composite_t;

void siera_ds_composite_init(
  siera_ds_composite_t* instance,
  siera_composite_route_t* routes,
  uint8_t count);

#endif
