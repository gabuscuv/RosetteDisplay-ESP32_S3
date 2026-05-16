idf_component_register(
    SRCS
        "mjpeg_player.c"
        "mjpeg_decoder.c"


    INCLUDE_DIRS
        "include"

    REQUIRES
        esp_common
        log
)