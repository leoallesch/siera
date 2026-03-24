#include "siera/sim.h"

#define GRID_COLUMNS 4

#define COLOR_BG         0x16161e
#define COLOR_SURFACE    0x1e1e2a
#define COLOR_SURFACE2   0x16161e
#define COLOR_BORDER     0x2e2e42
#define COLOR_BORDER_DIM 0x252535
#define COLOR_ACCENT     0x5B8AF0
#define COLOR_TEXT       0xd4d4e8
#define COLOR_TEXT_DIM   0x6868a0

static void on_button_event(lv_event_t* e)
{
  siera_sim_input_ctx_t* ctx = lv_event_get_user_data(e);
  bool pressed = (lv_event_get_code(e) == LV_EVENT_PRESSED);
  siera_ds_write(ctx->ds, ctx->key, &pressed);
}

static void on_switch_event(lv_event_t* e)
{
  siera_sim_input_ctx_t* ctx = lv_event_get_user_data(e);
  lv_obj_t* sw = lv_event_get_target(e);
  bool checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
  siera_ds_write(ctx->ds, ctx->key, &checked);
}

static void on_slider_event(lv_event_t* e)
{
  siera_sim_input_ctx_t* ctx = lv_event_get_user_data(e);
  lv_obj_t* slider = lv_event_get_target(e);
  int32_t val = lv_slider_get_value(slider);
  uint16_t value = (uint16_t)val;
  siera_ds_write(ctx->ds, ctx->key, &value);
}

static lv_obj_t* create_input_widget(
  lv_obj_t* parent,
  const siera_sim_input_t* cfg,
  void* ctx)
{
  lv_obj_t* cell = lv_obj_create(parent);
  lv_obj_set_scrollbar_mode(cell, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scroll_dir(cell, LV_DIR_NONE);
  lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(
    cell, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_size(cell, lv_pct(100 / GRID_COLUMNS), LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(cell, lv_color_hex(COLOR_SURFACE2), 0);
  lv_obj_set_style_bg_opa(cell, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(cell, 10, 0);
  lv_obj_set_style_border_width(cell, 1, 0);
  lv_obj_set_style_border_color(cell, lv_color_hex(COLOR_BORDER_DIM), 0);
  lv_obj_set_style_pad_all(cell, 12, 0);
  lv_obj_set_style_pad_row(cell, 10, 0);

  lv_obj_t* label = lv_label_create(cell);
  lv_label_set_text(label, siera_ds_key_name(cfg->key));
  lv_obj_set_style_text_color(label, lv_color_hex(COLOR_TEXT_DIM), 0);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);

  switch(cfg->type) {
    case SIERA_SIM_INPUT_BUTTON: {
      lv_obj_t* btn = lv_button_create(cell);
      lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_SURFACE), 0);
      lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
      lv_obj_set_style_border_width(btn, 1, 0);
      lv_obj_set_style_border_color(btn, lv_color_hex(COLOR_BORDER), 0);
      lv_obj_set_style_radius(btn, 6, 0);
      lv_obj_set_style_pad_ver(btn, 6, 0);
      lv_obj_set_style_pad_hor(btn, 14, 0);
      lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_ACCENT), LV_STATE_PRESSED);
      lv_obj_set_style_border_color(btn, lv_color_hex(COLOR_ACCENT), LV_STATE_PRESSED);
      lv_obj_t* btn_label = lv_label_create(btn);
      lv_label_set_text(btn_label, "press");
      lv_obj_set_style_text_color(btn_label, lv_color_hex(COLOR_TEXT), 0);
      lv_obj_set_style_text_color(btn_label, lv_color_hex(0xffffff), LV_STATE_PRESSED);
      lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_12, 0);
      lv_obj_add_event_cb(btn, on_button_event, LV_EVENT_PRESSED, ctx);
      lv_obj_add_event_cb(btn, on_button_event, LV_EVENT_RELEASED, ctx);
      break;
    }
    case SIERA_SIM_INPUT_SWITCH: {
      lv_obj_t* sw = lv_switch_create(cell);
      lv_obj_set_style_bg_color(sw, lv_color_hex(COLOR_BORDER), LV_PART_MAIN);
      lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, LV_PART_MAIN);
      lv_obj_set_style_bg_color(
        sw, lv_color_hex(COLOR_ACCENT), LV_PART_INDICATOR | LV_STATE_CHECKED);
      lv_obj_set_style_bg_color(sw, lv_color_hex(0xffffff), LV_PART_KNOB);
      lv_obj_add_event_cb(sw, on_switch_event, LV_EVENT_VALUE_CHANGED, ctx);
      break;
    }
    case SIERA_SIM_INPUT_SLIDER: {
      lv_obj_t* slider = lv_slider_create(cell);
      lv_obj_set_width(slider, lv_pct(90));
      lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_BORDER), LV_PART_MAIN);
      lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
      lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_ACCENT), LV_PART_INDICATOR);
      lv_obj_set_style_bg_color(slider, lv_color_hex(0xffffff), LV_PART_KNOB);
      lv_obj_set_style_border_color(slider, lv_color_hex(COLOR_ACCENT), LV_PART_KNOB);
      lv_obj_set_style_border_width(slider, 2, LV_PART_KNOB);
      lv_obj_add_event_cb(slider, on_slider_event, LV_EVENT_VALUE_CHANGED, ctx);
      break;
    }
  }
  return cell;
}

static lv_obj_t* create_input_panel(
  lv_obj_t* parent,
  const siera_sim_input_t* inputs,
  uint8_t input_count,
  siera_sim_t* self)
{
  lv_obj_t* panel = lv_obj_create(parent);
  lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scroll_dir(panel, LV_DIR_NONE);
  lv_obj_set_size(panel, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(
    panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_color(panel, lv_color_hex(COLOR_SURFACE), 0);
  lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(panel, 0, 0);
  lv_obj_set_style_pad_all(panel, 16, 0);
  lv_obj_set_style_pad_row(panel, 8, 0);
  lv_obj_set_style_pad_column(panel, 8, 0);

  for(uint8_t i = 0; i < input_count; i++) {
    create_input_widget(panel, &inputs[i], &self->input_ctx[i]);
  }
  return panel;
}

static lv_obj_t* create_content_area(lv_obj_t* parent, int width, int height)
{
  lv_obj_t* border = lv_obj_create(parent);
  lv_obj_set_scrollbar_mode(border, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scroll_dir(border, LV_DIR_NONE);
  lv_obj_set_size(border, width + 4, height + 4);
  lv_obj_set_style_bg_color(border, lv_color_hex(0x0a0a12), 0);
  lv_obj_set_style_bg_opa(border, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(border, 2, 0);
  lv_obj_set_style_border_color(border, lv_color_hex(COLOR_BORDER), 0);
  lv_obj_set_style_radius(border, 6, 0);
  lv_obj_set_style_pad_all(border, 2, 0);

  lv_obj_t* content = lv_obj_create(border);
  lv_obj_remove_style_all(content);
  lv_obj_set_size(content, width, height);
  lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scroll_dir(content, LV_DIR_NONE);

  return content;
}

static lv_obj_t* create_screen(
  int app_width,
  int app_height,
  const siera_sim_input_t* inputs,
  uint8_t input_count,
  siera_sim_t* self,
  lv_obj_t** out_content_area,
  lv_obj_t** out_input_panel)
{
  lv_obj_t* screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BG), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
  lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scroll_dir(screen, LV_DIR_NONE);

  lv_obj_t* main_col = lv_obj_create(screen);
  lv_obj_remove_style_all(main_col);
  lv_obj_set_scrollbar_mode(main_col, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scroll_dir(main_col, LV_DIR_NONE);
  lv_obj_set_size(main_col, lv_pct(100), lv_pct(100));
  lv_obj_set_flex_flow(main_col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(
    main_col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(main_col, 0, 0);
  lv_obj_set_style_pad_all(main_col, 0, 0);

  *out_content_area = create_content_area(main_col, app_width, app_height);
  *out_input_panel = create_input_panel(main_col, inputs, input_count, self);

  return screen;
}

void siera_sim_init(
  siera_sim_t*              self,
  const siera_sim_config_t* cfg,
  siera_ds_t*               ds)
{
  self->ds = ds;

  for(uint8_t i = 0; i < cfg->input_count; i++) {
    self->input_ctx[i].ds  = ds;
    self->input_ctx[i].key = cfg->inputs[i].key;
  }

  int padding = 24;

  self->display = lv_sdl_window_create(cfg->app_width + padding, cfg->app_height + padding);
  self->mouse   = lv_sdl_mouse_create();

  lv_obj_t* input_panel;
  lv_obj_t* screen = create_screen(
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
    cfg->app_width + padding,
    cfg->app_height + panel_height + padding);

  lv_screen_load(screen);

  siera_sim_display_init(&self->sim_display, self->content_area);
}

siera_hal_display_t* siera_sim_get_display(siera_sim_t* self)
{
  return &self->sim_display.interface;
}
