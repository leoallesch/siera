#ifndef DS_NULL_H
#define DS_NULL_H

#include "siera/ds.h"

typedef struct {
  i_siera_ds_t interface;
  siera_event_t on_change;
} siera_ds_null_t;

void siera_null_ds_init(siera_ds_null_t* instance);

#endif
