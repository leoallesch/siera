add_executable(esp32_led_blink.elf main.c)
target_link_libraries(esp32_led_blink.elf PRIVATE
    siera::esp32
    idf::freertos
    idf::spi_flash
)
idf_build_executable(esp32_led_blink.elf)
