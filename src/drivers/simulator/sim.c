#include "siera/ds.h"
#include "siera/sim.h"
#include "sim_ui.h"

#include <stddef.h>
#include <stdint.h>

#define SIM_WINDOW_PADDING 48

/* -------------------------------------------------------------------------
 * Stream API — GPIO
 * -------------------------------------------------------------------------*/

static siera_sim_input_ctx_t* find_ctx(siera_sim_t* sim, siera_ds_key_t key)
{
  for(uint8_t i = 0; i < SIERA_SIM_MAX_INPUTS; i++) {
    if(sim->input_ctx[i].sim && sim->input_ctx[i].key == key)
      return &sim->input_ctx[i];
  }
  return NULL;
}

static int gpio_read(void* ctx, siera_ds_key_t key, void* buf, size_t size)
{
  (void)size;
  siera_sim_t* sim = (siera_sim_t*)ctx;
  siera_sim_input_ctx_t* entry = find_ctx(sim, key);
  if(!entry)
    return -1;
  *(bool*)buf = entry->bool_val;
  return 0;
}

static int gpio_write(void* ctx, siera_ds_key_t key, const void* buf, size_t size)
{
  (void)size;
  siera_sim_t* sim = (siera_sim_t*)ctx;
  siera_sim_input_ctx_t* entry = find_ctx(sim, key);
  if(!entry)
    return -1;
  entry->bool_val = *(const bool*)buf;
  return 0;
}

static const siera_ds_stream_api_t gpio_api = { gpio_read, gpio_write };

/* -------------------------------------------------------------------------
 * Stream API — ADC
 * -------------------------------------------------------------------------*/

static int adc_read(void* ctx, siera_ds_key_t key, void* buf, size_t size)
{
  (void)size;
  siera_sim_t* sim = (siera_sim_t*)ctx;
  siera_sim_input_ctx_t* entry = find_ctx(sim, key);
  if(!entry)
    return -1;
  *(uint16_t*)buf = entry->uint16_val;
  return 0;
}

static int adc_write(void* ctx, siera_ds_key_t key, const void* buf, size_t size)
{
  (void)ctx;
  (void)key;
  (void)buf;
  (void)size;
  return -1; /* ADC is read-only */
}

static const siera_ds_stream_api_t adc_api = { adc_read, adc_write };

/* -------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------*/

void siera_sim_init(siera_sim_t* self, const siera_sim_config_t* cfg)
{
  self->gpio_stream.api = &gpio_api;
  self->gpio_stream.ctx = self;
  self->adc_stream.api = &adc_api;
  self->adc_stream.ctx = self;

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

siera_ds_stream_t* siera_sim_get_gpio_stream(siera_sim_t* self)
{
  return &self->gpio_stream;
}

siera_ds_stream_t* siera_sim_get_adc_stream(siera_sim_t* self)
{
  return &self->adc_stream;
}
