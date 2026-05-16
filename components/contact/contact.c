#include "contact.h"
#include "esp_log.h"

static const char *TAG = "CONTACT";

void mode_contact_show(const char *text)
{
    ESP_LOGI(TAG, "CONTACT MODE: %s", text);

    // later:
    // - use ESP QR code lib
    // - render to framebuffer/LVGL canvas
}