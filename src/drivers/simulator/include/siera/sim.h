#ifndef SIERA_SIM_H
#define SIERA_SIM_H

#include "siera/ds.h"
#include "siera/sim_display.h"
#include "siera/sim_ds_io.h"

typedef struct {
  int app_width;
  int app_height;

  const siera_sim_widget_t* widgets;
  uint8_t widget_count;
} siera_sim_config_t;

typedef struct {
  lv_display_t* display;
  lv_indev_t* mouse;
  lv_obj_t* content_area;

  siera_sim_display_t sim_display;
  siera_sim_ds_io_t io;
} siera_sim_t;

void siera_sim_init(siera_sim_t* self, const siera_sim_config_t* cfg);
void siera_sim_deinit(siera_sim_t* self);
siera_hal_display_t* siera_sim_get_display(siera_sim_t* self);
i_siera_ds_t* siera_sim_get_io(siera_sim_t* self);

/* Attach output-widget subscriptions to an external observed datastream
   (typically the composite). Call after composite init. */
void siera_sim_bind_outputs(siera_sim_t* self, i_siera_ds_t* observed);

#endif /* SIERA_SIM_H */
