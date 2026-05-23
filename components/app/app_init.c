#include "app_init.h"
#include "app_display.h"
#include "app_state.h"
#include "display.h"
#include "ui_router.h"
#include "esp_log.h"

static const char *TAG = "APP_INIT";

void app_init_system(void)
{
    ESP_LOGI(TAG, "System init");
    app_display_init();

    ui_router_init();
    app_state_init();

    app_state_set(APP_MODE_SAMPLE);
}