#ifndef SIERA_SIM_H
#define SIERA_SIM_H

#include "lvgl.h"
#include "siera/ds.h"
#include "siera/dsk.h"
#include "siera/event.h"
#include "siera/sim_display.h"

#ifndef SIERA_SIM_MAX_INPUTS
#define SIERA_SIM_MAX_INPUTS 32
#endif

typedef enum {
  SIERA_SIM_INPUT_BUTTON,
  SIERA_SIM_INPUT_SWITCH,
  SIERA_SIM_INPUT_SLIDER,
} siera_sim_input_type_t;

typedef struct {
  siera_dsk_t key;
  siera_sim_input_type_t type;
} siera_sim_input_t;

typedef struct {
  siera_dsk_t key;
  const void* val;
  size_t size;
} siera_sim_input_event_t;

typedef struct {
  int app_width;
  int app_height;

  const siera_sim_input_t* inputs;
  uint8_t input_count;
} siera_sim_config_t;

typedef struct siera_sim_t siera_sim_t;

typedef struct {
  siera_sim_t* sim;
  siera_dsk_t key;
  siera_sim_input_type_t type;
  bool bool_val;
  uint16_t uint16_val;
} siera_sim_input_ctx_t;

/* Concrete GPIO stream for the simulator */
typedef struct {
  i_siera_ds_t interface;
  siera_event_t on_change;
  siera_sim_t* sim;
} siera_sim_gpio_stream_t;

/* Concrete ADC stream for the simulator */
typedef struct {
  i_siera_ds_t interface;
  siera_event_t on_change;
  siera_sim_t* sim;
} siera_sim_adc_stream_t;

struct siera_sim_t {
  lv_display_t* display;
  lv_indev_t* mouse;
  lv_obj_t* content_area;

  siera_sim_display_t sim_display;

  siera_sim_gpio_stream_t gpio_stream;
  siera_sim_adc_stream_t adc_stream;

  siera_event_t input_event;

  siera_sim_input_ctx_t input_ctx[SIERA_SIM_MAX_INPUTS];
  uint8_t input_count;
};

void siera_sim_init(siera_sim_t* self, const siera_sim_config_t* cfg);
siera_hal_display_t* siera_sim_get_display(siera_sim_t* self);
i_siera_ds_t* siera_sim_get_gpio_stream(siera_sim_t* self);
i_siera_ds_t* siera_sim_get_adc_stream(siera_sim_t* self);

#endif /* SIERA_SIM_H */
