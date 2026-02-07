#include "hello_view.h"
#include "screens/hello_world_gen.h"

static void load(i_view_t* self)
{
  hello_view_t* view = (hello_view_t*)self;

  view->screen = hello_world_create();
  lv_screen_load(view->screen);
}

static void unload(i_view_t* self)
{
  hello_view_t* view = (hello_view_t*)self;

  if(view->screen) {
    lv_obj_delete(view->screen);
    view->screen = NULL;
  }
}

void hello_view_init(hello_view_t* self)
{
  self->interface.load = load;
  self->interface.unload = unload;
  self->screen = NULL;
}
