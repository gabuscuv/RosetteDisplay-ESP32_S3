#include "ui_router.h"
#include "contact.h"
#include "qr.h"
#include "cadizgamedev.h"
#include "mjpeg_player.h"
#include "esp_log.h"
#include "sample.h"

static const char *TAG = "UI_ROUTER";
void ui_router_init()
{
    ESP_LOGI(TAG, "UI Router init");
    // future: init UI system, load resources, etc.
}

void ui_router_switch(app_mode_t mode)
{
    ESP_LOGI(TAG, "Switching UI mode: %d", mode);

    switch (mode) {

        case APP_MODE_CONTACT:
            mjpeg_player_stop();
            mode_contact_show("Contact information");
            break;

        case APP_MODE_CADIZGAMEDEV:
            mjpeg_player_stop();
            mode_cadizgamedev_show();
            break;

        case APP_MODE_VIDEO:
            mode_contact_show("Video mode"); // stop other UI
            mjpeg_source_t src = {0};
            mjpeg_player_start(src);
            break;
        case APP_MODE_QR:
            mjpeg_player_stop();
            mode_qr_show(""); // later: pass text to encode
            break;
        case APP_MODE_SLEEP:
            mjpeg_player_stop();
            mode_contact_show("Sleep mode"); // placeholder
            break;
        case APP_MODE_SAMPLE:
            mjpeg_player_stop();
            mode_sample_show();
            break;
        default:
            break;
    }
}