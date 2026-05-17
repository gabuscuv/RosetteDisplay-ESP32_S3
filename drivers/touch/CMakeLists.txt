idf_component_register(
    SRCS
        "CST816.c"
        "esp_lcd_touch/esp_lcd_touch.c"

    INCLUDE_DIRS
        "."
        "esp_lcd_touch"

    REQUIRES
        esp_lcd
        esp_timer
        driver
        exio
        lcd
        lvgl
)