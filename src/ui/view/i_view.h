#pragma once

typedef struct i_view_t {
  void (*load)(struct i_view_t* self, void* screen);
  void (*unload)(struct i_view_t* self);
} i_view_t;
