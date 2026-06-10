idf_component_register(
    SRCS
        "mjpeg_player.c"
        "mjpeg_decoder.c"


    INCLUDE_DIRS
        "include"

    REQUIRES
        freertos
        sdcard
        esp_common
        log
        display
        driver
        esp_driver_spi
        esp_driver_sdspi
        tjpgd
)