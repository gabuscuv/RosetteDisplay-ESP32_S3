#include "display_lvgl.h"
#include "LVGL_Driver.h"
#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "DISPLAY_LVGL";
static TaskHandle_t lvgl_task_handle = NULL;
static void display_lvgl_task(void *arg);

typedef enum {
    LVGL_STATE_UNINITIALIZED = 0,
    LVGL_STATE_RUNNING,
    LVGL_STATE_SUSPENDED,
} lvgl_state_t;

static lvgl_state_t lvgl_state = LVGL_STATE_UNINITIALIZED;

static void lvgl_resume(void)
{
    if (lvgl_state == LVGL_STATE_RUNNING) {
        ESP_LOGI(TAG, "LVGL already running");
        return;
    }

    if (lvgl_state == LVGL_STATE_UNINITIALIZED) {
        ESP_LOGW(TAG, "LVGL resume requested before initialization");
        return;
    }

    lv_timer_enable(true);
    lvgl_state = LVGL_STATE_RUNNING;
    ESP_LOGI(TAG, "LVGL resumed");
}

static void display_lvgl_start_task(void)
{
    if (lvgl_task_handle) {
        return;
    }

    BaseType_t result = xTaskCreate(
        display_lvgl_task,
        "lvgl_task",
        4096,
        NULL,
        5,
        &lvgl_task_handle);

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LVGL task");
        lvgl_task_handle = NULL;
    }
}

static void lvgl_init(void)
{
    if (lvgl_state != LVGL_STATE_UNINITIALIZED) {
        return;
    }

    ESP_LOGI(TAG, "Initializing LVGL");
    LVGL_Init();
    lvgl_state = LVGL_STATE_RUNNING;
    display_lvgl_start_task();
    ESP_LOGI(TAG, "LVGL initialized and running");
}

void display_lvgl_init(void)
{
    lvgl_init();
}

void display_lvgl_enable(void)
{
    
    if (lvgl_state == LVGL_STATE_UNINITIALIZED) {
        lvgl_init();
        return;
    }

    if (lvgl_state == LVGL_STATE_SUSPENDED) {
        lvgl_resume();
        return;
    }

    ESP_LOGI(TAG, "LVGL already enabled");
}

void display_lvgl_overlay_enable(void)
{
    display_lvgl_enable();
    ESP_LOGI(TAG, "LVGL overlay enabled");
}

void display_lvgl_disable(void)
{
    if (lvgl_state != LVGL_STATE_RUNNING) {
        if (lvgl_state == LVGL_STATE_UNINITIALIZED) {
            ESP_LOGW(TAG, "LVGL disable requested before initialization");
        } else {
            ESP_LOGI(TAG, "LVGL already disabled");
        }
        return;
    }

    lv_timer_enable(false);
    vTaskDelay(pdMS_TO_TICKS(20));
    lvgl_state = LVGL_STATE_SUSPENDED;
    ESP_LOGI(TAG, "LVGL suspended (UI frozen, state preserved)");
}

static void display_lvgl_task(void *arg)
{
    (void)arg;
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
