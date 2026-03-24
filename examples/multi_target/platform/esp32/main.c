#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "siera/esp32_timesource.h"

#include "app.h"

void app_main(void)
{
  siera_timesource_t* timesource = siera_esp32_timesource_init();

  static siera_timer_mgr_t timers;
  siera_timer_mgr_init(&timers, timesource);

  app_init(&timers);

  printf("=== multi_target / ESP32 ===\n");

  while(1) {
    siera_timer_tick(&timers);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
