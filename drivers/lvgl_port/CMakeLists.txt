idf_component_register(
    SRCS
        "LVGL_Driver.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        lvgl
        lcd
)