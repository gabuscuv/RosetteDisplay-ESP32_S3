idf_component_register(
    SRCS
        "cadizgamedev.c"

    INCLUDE_DIRS
        "include"

    REQUIRES
        esp_common
        log
        lvgl
)