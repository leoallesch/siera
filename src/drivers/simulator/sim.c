#include "siera/ds.h"
#include "siera/sim.h"
#include "sim_ui.h"

#include <stddef.h>
#include <stdint.h>

#define SIM_WINDOW_PADDING 48

/* -------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------*/

static siera_sim_input_ctx_t* find_ctx(siera_sim_t* sim, siera_dsk_t key)
{
  for(uint8_t i = 0; i < SIERA_SIM_MAX_INPUTS; i++) {
    if(sim->input_ctx[i].sim && sim->input_ctx[i].key == key)
      return &sim->input_ctx[i];
  }
  return NULL;
}

/* -------------------------------------------------------------------------
 * Stream API — GPIO
 * -------------------------------------------------------------------------*/

static void gpio_read(i_siera_ds_t* interface, siera_dsk_t key, void* buf)
{
  siera_sim_t* sim = ((siera_sim_gpio_stream_t*)interface)->sim;
  siera_sim_input_ctx_t* entry = find_ctx(sim, key);
  if(!entry)
    return;
  *(bool*)buf = entry->bool_val;
}

static void gpio_write(i_siera_ds_t* interface, siera_dsk_t key, const void* buf)
{
  siera_sim_gpio_stream_t* stream = (siera_sim_gpio_stream_t*)interface;
  siera_sim_input_ctx_t* entry = find_ctx(stream->sim, key);
  if(!entry)
    return;
  bool new_val = *(const bool*)buf;
  if(entry->bool_val != new_val) {
    entry->bool_val = new_val;
    siera_ds_on_change_args_t args = { .key = key, .data = &entry->bool_val };
    siera_event_publish(&stream->on_change, &args);
  }
}

static bool gpio_contains(i_siera_ds_t* interface, siera_dsk_t key)
{
  siera_sim_t* sim = ((siera_sim_gpio_stream_t*)interface)->sim;
  return find_ctx(sim, key) != NULL;
}

static size_t gpio_size(i_siera_ds_t* interface, siera_dsk_t key)
{
  (void)interface;
  (void)key;
  return sizeof(bool);
}

static siera_event_t* gpio_on_change(i_siera_ds_t* interface)
{
  return &((siera_sim_gpio_stream_t*)interface)->on_change;
}

static const i_siera_ds_api_t gpio_api = {
  .read = gpio_read,
  .write = gpio_write,
  .contains = gpio_contains,
  .size = gpio_size,
  .on_change = gpio_on_change,
};

/* -------------------------------------------------------------------------
 * Stream API — ADC
 * -------------------------------------------------------------------------*/

static void adc_read(i_siera_ds_t* interface, siera_dsk_t key, void* buf)
{
  siera_sim_t* sim = ((siera_sim_adc_stream_t*)interface)->sim;
  siera_sim_input_ctx_t* entry = find_ctx(sim, key);
  if(!entry)
    return;
  *(uint16_t*)buf = entry->uint16_val;
}

static void adc_write(i_siera_ds_t* interface, siera_dsk_t key, const void* buf)
{
  (void)interface;
  (void)key;
  (void)buf;
}

static bool adc_contains(i_siera_ds_t* interface, siera_dsk_t key)
{
  siera_sim_t* sim = ((siera_sim_adc_stream_t*)interface)->sim;
  return find_ctx(sim, key) != NULL;
}

static size_t adc_size(i_siera_ds_t* interface, siera_dsk_t key)
{
  (void)interface;
  (void)key;
  return sizeof(uint16_t);
}

static siera_event_t* adc_on_change(i_siera_ds_t* interface)
{
  return &((siera_sim_adc_stream_t*)interface)->on_change;
}

static const i_siera_ds_api_t adc_api = {
  .read = adc_read,
  .write = adc_write,
  .contains = adc_contains,
  .size = adc_size,
  .on_change = adc_on_change,
};

/* -------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------*/

void siera_sim_init(siera_sim_t* self, const siera_sim_config_t* cfg)
{
  self->gpio_stream.interface.api = &gpio_api;
  self->gpio_stream.on_change = (siera_event_t){ 0 };
  siera_event_init(&self->gpio_stream.on_change);
  self->gpio_stream.sim = self;

  self->adc_stream.interface.api = &adc_api;
  self->adc_stream.on_change = (siera_event_t){ 0 };
  siera_event_init(&self->adc_stream.on_change);
  self->adc_stream.sim = self;

  siera_event_init(&self->input_event);

  for(uint8_t i = 0; i < cfg->input_count; i++) {
    self->input_ctx[i].sim = self;
    self->input_ctx[i].key = cfg->inputs[i].key;
    self->input_ctx[i].type = cfg->inputs[i].type;
    self->input_ctx[i].bool_val = false;
    self->input_ctx[i].uint16_val = 0;
  }
  self->input_count = cfg->input_count;

  int panel_width = sim_ui_panel_width(cfg->input_count);
  int window_width = cfg->app_width > panel_width ? cfg->app_width : panel_width;

  self->display = lv_sdl_window_create(
    window_width + SIM_WINDOW_PADDING,
    cfg->app_height + SIM_WINDOW_PADDING);
  self->mouse = lv_sdl_mouse_create();

  lv_obj_t* input_panel;
  lv_obj_t* screen = sim_ui_create_screen(
    cfg->app_width,
    cfg->app_height,
    cfg->inputs,
    cfg->input_count,
    self,
    &self->content_area,
    &input_panel);

  lv_obj_update_layout(input_panel);
  int panel_height = lv_obj_get_height(input_panel);
  lv_display_set_resolution(
    self->display,
    window_width + SIM_WINDOW_PADDING,
    cfg->app_height + panel_height + SIM_WINDOW_PADDING);

  lv_screen_load(screen);

  siera_sim_display_init(&self->sim_display, self->content_area);
}

siera_hal_display_t* siera_sim_get_display(siera_sim_t* self)
{
  return &self->sim_display.interface;
}

i_siera_ds_t* siera_sim_get_gpio_stream(siera_sim_t* self)
{
  return &self->gpio_stream.interface;
}

i_siera_ds_t* siera_sim_get_adc_stream(siera_sim_t* self)
{
  return &self->adc_stream.interface;
}
