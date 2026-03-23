siera_add_module(
    NAME siera_drv_esp32
    SRCS hal_timesource.c hal_gpio.c hal_uart.c hal_adc.c hal_pwm.c
         ds_gpio.c ds_adc.c)
