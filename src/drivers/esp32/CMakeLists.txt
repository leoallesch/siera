# ── HAL implementations ────────────────────────────────────────────────────────
# Implement siera/hal_*.h interfaces for ESP-IDF peripherals.

# ── DS stream implementations ──────────────────────────────────────────────────
# Implement siera_ds_stream_t backed by HAL drivers (GPIO, ADC).

siera_add_module(
    SRCS    hal/hal_timesource.c
            hal/hal_gpio.c
            hal/hal_uart.c
            hal/hal_adc.c
            hal/hal_pwm.c
            ds/ds_gpio.c
            ds/ds_adc.c
    INCLUDES include
    LINKS   idf::esp_driver_gpio 
            idf::esp_driver_uart 
            idf::esp_driver_ledc 
            idf::esp_timer 
            idf::esp_adc
)
