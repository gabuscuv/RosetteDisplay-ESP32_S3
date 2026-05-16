#include "app_display.h"
#include "esp_log.h"

static const char *TAG = "APP_DISPLAY";

void app_display_init(void)
{
    ESP_LOGI(TAG, "Init display (LCD + driver + LVGL later)");
}

void app_display_clear(void)
{
    ESP_LOGI(TAG, "Clear screen");
}

void app_display_flush(void)
{
    ESP_LOGI(TAG, "Flush frame buffer");
}