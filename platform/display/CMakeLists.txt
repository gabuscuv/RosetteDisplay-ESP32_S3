idf_component_register(
    SRCS
        "display.c"
        "display_lvgl.c"

    INCLUDE_DIRS
        "include"

    REQUIRES
        driver
        lcd
        lvgl_port
)