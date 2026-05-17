idf_component_register(
    SRCS
        "ST77916.c"
        "esp_lcd_st77916/esp_lcd_st77916.c"

    INCLUDE_DIRS
        "."
        "esp_lcd_st77916"

    REQUIRES
        esp_lcd
        touch
        driver
        lvgl_port
)