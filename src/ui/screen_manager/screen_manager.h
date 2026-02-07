#pragma once

#include "i_view.h"

typedef struct {
  i_view_t* active;
} screen_manager_t;

void screen_manager_init(screen_manager_t* instance);
void screen_manager_show(screen_manager_t* instance, i_view_t* view);
i_view_t* screen_manager_get_active(screen_manager_t* instance);
