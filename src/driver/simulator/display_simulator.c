#include "display_simulator.h"
#include "event_subscription.h"
#include "i_datastream.h"
#include "i_view.h"

#define GRID_COLUMNS 4
#define COLOR_BG 0x0f0f1a
#define COLOR_SURFACE 0x1a1a2e
#define COLOR_BORDER 0x2a2a4a
#define COLOR_ACCENT 0x4a6cf7
#define COLOR_TEXT 0xe0e0e0
#define COLOR_TEXT_DIM 0x888899

typedef struct {
  i_datastream_t* stream;
  datastream_key_t key;
} sim_input_ctx_t;

static void on_button_event(lv_event_t* e)
{
  sim_input_ctx_t* ctx = lv_event_get_user_data(e);
  bool pressed = (lv_event_get_code(e) == LV_EVENT_PRESSED);
  datastream_write(ctx->stream, ctx->key, &pressed);
}

static void on_switch_event(lv_event_t* e)
{
  sim_input_ctx_t* ctx = lv_event_get_user_data(e);
  lv_obj_t* sw = lv_event_get_target(e);
  bool checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
  datastream_write(ctx->stream, ctx->key, &checked);
}

static void on_slider_event(lv_event_t* e)
{
  sim_input_ctx_t* ctx = lv_event_get_user_data(e);
  lv_obj_t* slider = lv_event_get_target(e);
  int32_t val = lv_slider_get_value(slider);
  uint16_t value = (uint16_t)val;
  datastream_write(ctx->stream, ctx->key, &value);
}

static lv_obj_t* create_input_widget(
  lv_obj_t* parent,
  const simulator_input_config_t* cfg,
  sim_input_ctx_t* ctx)
{
  lv_obj_t* cell = lv_obj_create(parent);
  lv_obj_remove_style_all(cell);
  lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(
    cell, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_size(cell, lv_pct(100 / GRID_COLUMNS), LV_SIZE_CONTENT);
  lv_obj_set_style_pad_row(cell, 6, 0);
  lv_obj_set_style_pad_all(cell, 10, 0);
  lv_obj_set_style_bg_color(cell, lv_color_hex(COLOR_SURFACE), 0);
  lv_obj_set_style_bg_opa(cell, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(cell, 8, 0);
  lv_obj_set_style_border_width(cell, 1, 0);
  lv_obj_set_style_border_color(cell, lv_color_hex(COLOR_BORDER), 0);

  lv_obj_t* label = lv_label_create(cell);
  lv_label_set_text(label, cfg->name);
  lv_obj_set_style_text_color(label, lv_color_hex(COLOR_TEXT_DIM), 0);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);

  switch(cfg->input) {
    case SIMULATOR_INPUT_BUTTON: {
      lv_obj_t* btn = lv_button_create(cell);
      lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_ACCENT), 0);
      lv_obj_set_style_radius(btn, 6, 0);
      lv_obj_set_style_pad_ver(btn, 6, 0);
      lv_obj_set_style_pad_hor(btn, 16, 0);
      lv_obj_t* btn_label = lv_label_create(btn);
      lv_label_set_text(btn_label, "Press");
      lv_obj_set_style_text_color(btn_label, lv_color_hex(0xffffff), 0);
      lv_obj_add_event_cb(btn, on_button_event, LV_EVENT_PRESSED, ctx);
      lv_obj_add_event_cb(btn, on_button_event, LV_EVENT_RELEASED, ctx);
      break;
    }
    case SIMULATOR_INPUT_SWITCH: {
      lv_obj_t* sw = lv_switch_create(cell);
      lv_obj_set_style_bg_color(sw, lv_color_hex(COLOR_BORDER), LV_PART_MAIN);
      lv_obj_set_style_bg_color(
        sw, lv_color_hex(COLOR_ACCENT), LV_PART_INDICATOR | LV_STATE_CHECKED);
      lv_obj_add_event_cb(sw, on_switch_event, LV_EVENT_VALUE_CHANGED, ctx);
      break;
    }
    case SIMULATOR_INPUT_SLIDER: {
      lv_obj_t* slider = lv_slider_create(cell);
      lv_obj_set_width(slider, lv_pct(90));
      lv_obj_set_style_bg_color(
        slider, lv_color_hex(COLOR_BORDER), LV_PART_MAIN);
      lv_obj_set_style_bg_color(
        slider, lv_color_hex(COLOR_ACCENT), LV_PART_INDICATOR);
      lv_obj_set_style_bg_color(
        slider, lv_color_hex(0xffffff), LV_PART_KNOB);
      lv_obj_add_event_cb(slider, on_slider_event, LV_EVENT_VALUE_CHANGED, ctx);
      break;
    }
  }
  return cell;
}

static lv_obj_t* create_input_panel(
  lv_obj_t* parent,
  const simulator_input_config_t* input_config,
  uint8_t input_config_count,
  sim_input_ctx_t* contexts)
{
  lv_obj_t* panel = lv_obj_create(parent);
  lv_obj_remove_style_all(panel);
  lv_obj_set_size(panel, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(
    panel,
    LV_FLEX_ALIGN_CENTER,
    LV_FLEX_ALIGN_CENTER,
    LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(panel, 8, 0);
  lv_obj_set_style_pad_row(panel, 8, 0);
  lv_obj_set_style_pad_column(panel, 8, 0);

  for(uint8_t i = 0; i < input_config_count; i++) {
    create_input_widget(panel, &input_config[i], &contexts[i]);
  }
  return panel;
}

static int calculate_input_panel_height(lv_obj_t* panel)
{
  lv_obj_update_layout(panel);
  return lv_obj_get_height(panel);
}

static lv_obj_t* create_content_area(lv_obj_t* parent, int width, int height)
{
  lv_obj_t* border = lv_obj_create(parent);
  lv_obj_remove_style_all(border);
  lv_obj_set_size(border, width + 4, height + 4);
  lv_obj_set_style_border_width(border, 2, 0);
  lv_obj_set_style_border_color(border, lv_color_hex(COLOR_BORDER), 0);
  lv_obj_set_style_radius(border, 4, 0);
  lv_obj_set_style_pad_all(border, 2, 0);

  lv_obj_t* content = lv_obj_create(border);
  lv_obj_remove_style_all(content);
  lv_obj_set_size(content, width, height);

  return content;
}

static lv_obj_t* create_screen(
  int app_width,
  int app_height,
  const simulator_input_config_t* input_config,
  uint8_t input_config_count,
  sim_input_ctx_t* contexts,
  lv_obj_t** out_content_area,
  lv_obj_t** out_input_panel)
{
  lv_obj_t* screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BG), 0);

  lv_obj_t* main_col = lv_obj_create(screen);
  lv_obj_remove_style_all(main_col);
  lv_obj_set_size(main_col, lv_pct(100), lv_pct(100));
  lv_obj_set_flex_flow(main_col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(
    main_col,
    LV_FLEX_ALIGN_CENTER,
    LV_FLEX_ALIGN_CENTER,
    LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(main_col, 16, 0);
  lv_obj_set_style_pad_all(main_col, 12, 0);

  *out_content_area = create_content_area(main_col, app_width, app_height);
  *out_input_panel =
    create_input_panel(main_col, input_config, input_config_count, contexts);

  return screen;
}

static void on_view_change(void* context, const void* _args)
{
  display_simulator_t* instance = (display_simulator_t*)context;
  const datastream_on_change_args_t* args = (const datastream_on_change_args_t*)_args;

  i_view_t* view = *(i_view_t* const*)args->data;

  if(instance->current_view) {
    instance->current_view->unload(instance->current_view);
  }

  if(view) {
    instance->current_view = view;
    view->load(view, instance->content_area);
  }
}

void display_simulator_init(
  display_simulator_t* instance,
  int app_width,
  int app_height,
  const simulator_input_config_t* input_config,
  uint8_t input_config_count,
  i_datastream_t* bsp_stream,
  i_datastream_t* ram_stream,
  datastream_key_t current_view)
{
  instance->current_view = NULL;
  instance->bsp_stream = bsp_stream;
  instance->ram_stream = ram_stream;

  // Allocate context structs for LVGL event callbacks
  static sim_input_ctx_t contexts[32];
  for(uint8_t i = 0; i < input_config_count; i++) {
    contexts[i].stream = bsp_stream;
    contexts[i].key = input_config[i].key;
  }

  int window_width = app_width * 2;
  int padding = 24;

  instance->sdl_display = lv_sdl_window_create(window_width + padding, app_height + padding);
  instance->indev = lv_sdl_mouse_create();

  lv_obj_t* input_panel;
  lv_obj_t* screen = create_screen(
    app_width,
    app_height,
    input_config,
    input_config_count,
    contexts,
    &instance->content_area,
    &input_panel);

  int panel_height = calculate_input_panel_height(input_panel);
  lv_display_set_resolution(
    instance->sdl_display,
    window_width + padding,
    app_height + panel_height + padding);

  lv_screen_load(screen);

  event_subscription_init(
    &instance->sub,
    on_view_change,
    instance);

  datastream_subscribe(ram_stream, current_view, &instance->sub);
}
