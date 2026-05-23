idf_component_register(
    SRCS
        "sample.c"
        "ui_router.c"

    INCLUDE_DIRS
        "include"

    REQUIRES
        lvgl_config
        app
        qr
        cadizgamedev
        contact
        video
        esp_common
        log
)