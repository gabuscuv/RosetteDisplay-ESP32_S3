#include "qr.h"
#include "esp_log.h"

static const char *TAG = "QR";

void mode_qr_show(const char *text)
{
    ESP_LOGI(TAG, "QR MODE: %s", text);

    // later:
    // - use ESP QR code lib
    // - render to framebuffer/LVGL canvas
}