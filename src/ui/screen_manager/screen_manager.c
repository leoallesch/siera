#include "screen_manager.h"

#include <stddef.h>

void screen_manager_init(screen_manager_t* instance)
{
  instance->active = NULL;
}

void screen_manager_show(screen_manager_t* instance, i_view_t* view)
{
  if(instance->active) {
    instance->active->unload(instance->active);
  }

  instance->active = view;

  if(instance->active) {
    instance->active->load(instance->active);
  }
}

i_view_t* screen_manager_get_active(screen_manager_t* instance)
{
  return instance->active;
}
