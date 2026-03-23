#include "siera/esp32_pwm.h"

#define PWM_TIMER      LEDC_TIMER_0
#define PWM_FREQ_HZ    5000
#define PWM_RESOLUTION LEDC_TIMER_10_BIT
#define PWM_MAX_DUTY   ((1u << 10) - 1) /* 1023 */

typedef struct {
  siera_hal_pwm_t interface;
  ledc_mode_t     speed_mode;
} siera_esp32_pwm_t;

static siera_esp32_pwm_t s_pwm;

static int _set_duty(siera_hal_pwm_t* self, uint32_t channel, uint32_t duty_permil)
{
  siera_esp32_pwm_t* drv = (siera_esp32_pwm_t*)self;
  if(duty_permil > 1000) duty_permil = 1000;
  uint32_t duty = (duty_permil * PWM_MAX_DUTY) / 1000;
  ledc_set_duty(drv->speed_mode, (ledc_channel_t)channel, duty);
  ledc_update_duty(drv->speed_mode, (ledc_channel_t)channel);
  return 0;
}

static int _enable(siera_hal_pwm_t* self, uint32_t channel)
{
  siera_esp32_pwm_t* drv = (siera_esp32_pwm_t*)self;
  ledc_channel_config_t cfg = {
    .channel    = (ledc_channel_t)channel,
    .duty       = 0,
    .gpio_num   = -1, /* caller must configure GPIO separately */
    .speed_mode = drv->speed_mode,
    .hpoint     = 0,
    .timer_sel  = PWM_TIMER,
  };
  return ledc_channel_config(&cfg) == ESP_OK ? 0 : -1;
}

static int _disable(siera_hal_pwm_t* self, uint32_t channel)
{
  siera_esp32_pwm_t* drv = (siera_esp32_pwm_t*)self;
  ledc_set_duty(drv->speed_mode, (ledc_channel_t)channel, 0);
  ledc_update_duty(drv->speed_mode, (ledc_channel_t)channel);
  return 0;
}

siera_hal_pwm_t* siera_esp32_pwm_init(ledc_mode_t speed_mode)
{
  s_pwm.speed_mode = speed_mode;

  ledc_timer_config_t timer_cfg = {
    .speed_mode      = speed_mode,
    .timer_num       = PWM_TIMER,
    .duty_resolution = PWM_RESOLUTION,
    .freq_hz         = PWM_FREQ_HZ,
    .clk_cfg         = LEDC_AUTO_CLK,
  };
  ledc_timer_config(&timer_cfg);

  s_pwm.interface.set_duty = _set_duty;
  s_pwm.interface.enable   = _enable;
  s_pwm.interface.disable  = _disable;
  return &s_pwm.interface;
}
