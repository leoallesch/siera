#include <stdio.h>

#include "lvgl.h"
#include "screen_manager.h"
#include "hello_view.h"
#include "ui_hello_world.h"

#define WINDOW_WIDTH  480
#define WINDOW_HEIGHT 320

int main(void)
{
  printf("Starting Simulator Example\n");

  lv_init();

  lv_display_t* display = lv_sdl_window_create(WINDOW_WIDTH, WINDOW_HEIGHT);
  (void)display;

  lv_indev_t* mouse = lv_sdl_mouse_create();
  (void)mouse;

  ui_hello_world_init(NULL);

  screen_manager_t screen_manager;
  screen_manager_init(&screen_manager);

  hello_view_t hello_view;
  hello_view_init(&hello_view);

  // screen_manager_show(&screen_manager, &hello_view.interface);

  while(1) {
    uint32_t time_till_next = lv_timer_handler();
    lv_delay_ms(time_till_next);
  }

  return 0;
}
