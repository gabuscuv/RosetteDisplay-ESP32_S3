#include "app_state.h"
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

    ui_router_switch(mode);
}

void app_state_tick(void)
{
    // future: timeout transitions, animations, input handling
}