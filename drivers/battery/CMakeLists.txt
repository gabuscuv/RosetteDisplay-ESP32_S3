idf_component_register(
    SRCS
        "BAT_Driver.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        driver
        esp_adc
)