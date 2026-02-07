#pragma once

#include "i_view.h"
#include "lvgl.h"

typedef struct {
  i_view_t interface;
  lv_obj_t* screen;
} hello_view_t;

void hello_view_init(hello_view_t* self);
