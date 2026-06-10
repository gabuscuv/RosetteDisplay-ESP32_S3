#include "app_state.h"
#include "display_mode_t.h"
#include "app_display.h"
#include "ui_router.h"
#include "esp_log.h"

static const char *TAG = "APP_STATE";

static app_mode_t current_mode;

void app_state_init(void)
{
    current_mode = APP_MODE_NONE; // default mode
}

app_mode_t app_state_get(void)
{
    return current_mode;
}

void app_state_set(app_mode_t mode)
{
    if (mode == current_mode) return;

    ESP_LOGI(TAG, "Switching mode: %d -> %d", current_mode, mode);
    current_mode = mode;
    switch (mode) {
        case APP_MODE_QR:
        case APP_MODE_SLEEP:
        case APP_MODE_SAMPLE:
        case APP_MODE_CONTACT:
        case APP_MODE_CADIZGAMEDEV:
            app_display_switch_mode(DISPLAY_MODE_LVGL);
            break;
        case APP_MODE_VIDEO:
            app_display_switch_mode(DISPLAY_MODE_RAW);
            break;
        default:
            ESP_LOGW(TAG, "Unknown mode: %d", mode);
            return;
    }

    ui_router_switch(mode);
}

void app_state_tick(void)
{
    // future: timeout transitions, animations, input handling
}