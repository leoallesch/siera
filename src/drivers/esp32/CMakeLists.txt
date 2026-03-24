add_library(siera_drv_esp32 STATIC
    hal_timesource.c
    hal_gpio.c
    hal_uart.c
    hal_adc.c
    hal_pwm.c
    ds_gpio.c
    ds_adc.c
)
add_library(siera::esp32 ALIAS siera_drv_esp32)

target_include_directories(siera_drv_esp32 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(siera_drv_esp32 PUBLIC
    siera
    idf::esp_driver_gpio
    idf::esp_driver_uart
    idf::esp_driver_ledc
    idf::esp_timer
    idf::esp_adc
)
