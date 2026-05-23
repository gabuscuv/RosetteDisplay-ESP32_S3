#include "sample.h"
#include "esp_log.h"
#include "lvgl.h"

static const char *TAG = "SAMPLE";

void mode_sample_show(void)
{
    ESP_LOGI(TAG, "SAMPLE MODE: Hello World");

    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello World");
    lv_obj_center(label);
}