#include "tjpgd.h"
#include "mjpeg_player.h"
#include "esp_log.h"

static const char *TAG = "MJPEG";

static bool running = false;

void mjpeg_player_start(mjpeg_source_t src)
{
    ESP_LOGI(TAG, "Starting MJPEG video mode");

    running = true;

    // later:
    // - create FreeRTOS task
    // - decode frames
    // - push to LCD/LVGL buffer
}

void mjpeg_player_stop(void)
{
    ESP_LOGI(TAG, "Stopping MJPEG video mode");
    running = false;
}

bool mjpeg_player_is_running(void)
{
    return running;
}