#ifndef SIERA_SIM_DS_IO_H
#define SIERA_SIM_DS_IO_H

#include "lvgl.h"
#include "siera/ds.h"
#include "siera/dsk.h"
#include "siera/event.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  /* Inputs — user-driven; widget callbacks publish to on_change */
  SIERA_SIM_WIDGET_BUTTON,  /* bool     */
  SIERA_SIM_WIDGET_SWITCH,  /* bool     */
  SIERA_SIM_WIDGET_SLIDER,  /* uint16_t */
  /* Outputs — subscribed to an observed ds; redraw on change */
  SIERA_SIM_WIDGET_LED,     /* bool     */
  SIERA_SIM_WIDGET_NUMERIC, /* uint16_t */
} siera_sim_widget_type_t;

typedef struct {
  siera_dsk_t key;
  siera_sim_widget_type_t type;
  const char* label; /* optional; NULL = no label */
} siera_sim_widget_t;

typedef struct siera_sim_ds_io_t siera_sim_ds_io_t;

typedef struct {
  siera_sim_ds_io_t* owner;
  siera_dsk_t key;
  siera_sim_widget_type_t type;
  bool bool_val;
  uint16_t uint16_val;
  lv_obj_t* widget; /* set by sim_ui after creation */
  siera_event_sub_t sub; /* used by outputs only */
} siera_sim_widget_ctx_t;

struct siera_sim_ds_io_t {
  i_siera_ds_t interface;
  siera_event_t on_change;
  siera_sim_widget_ctx_t* ctx; /* heap-allocated, length = count */
  uint8_t count;
};

void siera_sim_ds_io_init(
  siera_sim_ds_io_t* self,
  const siera_sim_widget_t* widgets,
  uint8_t count);
void siera_sim_ds_io_deinit(siera_sim_ds_io_t* self);

/* Attach output-widget subscriptions to an external observed datastream
   (typically the composite). Must be called after the composite is built. */
void siera_sim_ds_io_bind_outputs(siera_sim_ds_io_t* self, i_siera_ds_t* observed);

static inline bool siera_sim_widget_is_output(siera_sim_widget_type_t t)
{
  return t == SIERA_SIM_WIDGET_LED || t == SIERA_SIM_WIDGET_NUMERIC;
}

static inline bool siera_sim_widget_is_bool(siera_sim_widget_type_t t)
{
  return t == SIERA_SIM_WIDGET_BUTTON || t == SIERA_SIM_WIDGET_SWITCH
      || t == SIERA_SIM_WIDGET_LED;
}

#endif /* SIERA_SIM_DS_IO_H */
