#include "siera/sim_ds_io.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"

static siera_sim_widget_ctx_t* find_ctx(siera_sim_ds_io_t* self, siera_dsk_t key)
{
  for(uint8_t i = 0; i < self->count; i++) {
    if(self->ctx[i].key == key)
      return &self->ctx[i];
  }
  return NULL;
}

/* -------------------------------------------------------------------------
 * Output rendering
 * -------------------------------------------------------------------------*/

static void render_output(siera_sim_widget_ctx_t* ctx)
{
  if(!ctx->widget)
    return;

  switch(ctx->type) {
    case SIERA_SIM_WIDGET_LED: {
      lv_color_t color = ctx->bool_val ? lv_color_hex(0xff5e6c) : lv_color_hex(0x2e2e42);
      lv_obj_set_style_bg_color(ctx->widget, color, 0);
      break;
    }
    case SIERA_SIM_WIDGET_NUMERIC: {
      char buf[16];
      lv_snprintf(buf, sizeof(buf), "%u", (unsigned)ctx->uint16_val);
      lv_label_set_text(ctx->widget, buf);
      break;
    }
    default:
      break;
  }
}

static void on_observed_change(void* context, const void* args)
{
  siera_sim_widget_ctx_t* ctx = (siera_sim_widget_ctx_t*)context;
  const siera_ds_on_change_args_t* a = (const siera_ds_on_change_args_t*)args;

  if(a->key != ctx->key)
    return;

  if(siera_sim_widget_is_bool(ctx->type))
    ctx->bool_val = *(const bool*)a->data;
  else
    ctx->uint16_val = *(const uint16_t*)a->data;

  render_output(ctx);
}

/* -------------------------------------------------------------------------
 * i_siera_ds_t implementation
 * -------------------------------------------------------------------------*/

static void io_read(i_siera_ds_t* interface, siera_dsk_t key, void* buf)
{
  siera_sim_ds_io_t* self = (siera_sim_ds_io_t*)interface;
  siera_sim_widget_ctx_t* ctx = find_ctx(self, key);
  if(!ctx)
    return;
  if(siera_sim_widget_is_bool(ctx->type))
    *(bool*)buf = ctx->bool_val;
  else
    *(uint16_t*)buf = ctx->uint16_val;
}

static void io_write(i_siera_ds_t* interface, siera_dsk_t key, const void* buf)
{
  siera_sim_ds_io_t* self = (siera_sim_ds_io_t*)interface;
  siera_sim_widget_ctx_t* ctx = find_ctx(self, key);
  if(!ctx)
    return;

  bool changed = false;
  if(siera_sim_widget_is_bool(ctx->type)) {
    bool v = *(const bool*)buf;
    if(ctx->bool_val != v) {
      ctx->bool_val = v;
      changed = true;
    }
  }
  else {
    uint16_t v = *(const uint16_t*)buf;
    if(ctx->uint16_val != v) {
      ctx->uint16_val = v;
      changed = true;
    }
  }

  if(!changed)
    return;

  if(siera_sim_widget_is_output(ctx->type))
    render_output(ctx);

  const void* data = siera_sim_widget_is_bool(ctx->type)
                       ? (const void*)&ctx->bool_val
                       : (const void*)&ctx->uint16_val;
  siera_ds_on_change_args_t args = { .key = key, .data = data };
  siera_event_publish(&self->on_change, &args);
}

static bool io_contains(i_siera_ds_t* interface, siera_dsk_t key)
{
  siera_sim_ds_io_t* self = (siera_sim_ds_io_t*)interface;
  siera_sim_widget_ctx_t* ctx = find_ctx(self, key);
  /* Outputs mirror keys owned by other streams — they should not claim them
     in the composite routing table, otherwise reads/writes from the app
     would land on the mirror instead of the source. */
  return ctx && !siera_sim_widget_is_output(ctx->type);
}

static size_t io_size(i_siera_ds_t* interface, siera_dsk_t key)
{
  siera_sim_ds_io_t* self = (siera_sim_ds_io_t*)interface;
  siera_sim_widget_ctx_t* ctx = find_ctx(self, key);
  if(!ctx)
    return 0;
  return siera_sim_widget_is_bool(ctx->type) ? sizeof(bool) : sizeof(uint16_t);
}

static siera_event_t* io_on_change(i_siera_ds_t* interface)
{
  return &((siera_sim_ds_io_t*)interface)->on_change;
}

static const i_siera_ds_api_t _api = {
  .read = io_read,
  .write = io_write,
  .contains = io_contains,
  .size = io_size,
  .on_change = io_on_change,
};

/* -------------------------------------------------------------------------
 * Public init
 * -------------------------------------------------------------------------*/

void siera_sim_ds_io_init(
  siera_sim_ds_io_t* self,
  const siera_sim_widget_t* widgets,
  uint8_t count)
{
  self->interface.api = &_api;
  siera_event_init(&self->on_change);

  self->count = count;
  self->ctx = count ? calloc(count, sizeof(*self->ctx)) : NULL;
  for(uint8_t i = 0; i < count; i++) {
    self->ctx[i].owner = self;
    self->ctx[i].key = widgets[i].key;
    self->ctx[i].type = widgets[i].type;
  }
}

void siera_sim_ds_io_deinit(siera_sim_ds_io_t* self)
{
  free(self->ctx);
  self->ctx = NULL;
  self->count = 0;
}

void siera_sim_ds_io_bind_outputs(siera_sim_ds_io_t* self, i_siera_ds_t* observed)
{
  for(uint8_t i = 0; i < self->count; i++) {
    siera_sim_widget_ctx_t* ctx = &self->ctx[i];
    if(!siera_sim_widget_is_output(ctx->type))
      continue;
    siera_event_sub_init(&ctx->sub, on_observed_change, ctx);
    siera_ds_subscribe_all(observed, &ctx->sub);
  }
}
