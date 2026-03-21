idf_component_register(
    SRCS            "esp32_flash_kv.c"
    INCLUDE_DIRS    "."
    REQUIRES        nvs_flash siera
)
