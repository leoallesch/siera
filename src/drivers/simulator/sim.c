#include "siera/sim.h"
#include "siera/sim_ui.h"

#include <stddef.h>
#include <stdint.h>

#define SIM_WINDOW_PADDING 48

void siera_sim_init(siera_sim_t* self, const siera_sim_config_t* cfg)
{
  siera_sim_ds_io_init(&self->io, cfg->widgets, cfg->widget_count);

  int panel_width = sim_ui_panel_width(cfg->widget_count);
  int window_width = cfg->app_width > panel_width ? cfg->app_width : panel_width;

  self->display = lv_sdl_window_create(
    window_width + SIM_WINDOW_PADDING,
    cfg->app_height + SIM_WINDOW_PADDING);
  self->mouse = lv_sdl_mouse_create();

  lv_obj_t* input_panel;
  lv_obj_t* screen = sim_ui_create_screen(
    cfg->app_width,
    cfg->app_height,
    cfg->widgets,
    cfg->widget_count,
    &self->io,
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

void siera_sim_deinit(siera_sim_t* self)
{
  siera_sim_ds_io_deinit(&self->io);
}

siera_hal_display_t* siera_sim_get_display(siera_sim_t* self)
{
  return &self->sim_display.interface;
}

i_siera_ds_t* siera_sim_get_io(siera_sim_t* self)
{
  return &self->io.interface;
}

void siera_sim_bind_outputs(siera_sim_t* self, i_siera_ds_t* observed)
{
  siera_sim_ds_io_bind_outputs(&self->io, observed);
}
