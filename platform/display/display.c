#include "display.h"
#include "display_lvgl.h"
#include "esp_log.h"

// vendor drivers
#include "ST77916.h"

static const char *TAG = "DISPLAY";
static display_mode_t current_mode = DISPLAY_MODE_NULL;

static SemaphoreHandle_t lcd_done = NULL;

static bool on_color_trans_done(
    esp_lcd_panel_io_handle_t panel_io,
    esp_lcd_panel_io_event_data_t *edata,
    void *user_ctx)
{
    BaseType_t hp = pdFALSE;

    if (lcd_done) {
        xSemaphoreGiveFromISR(lcd_done, &hp);
    }

    return hp == pdTRUE;
}

void display_init(void)
{
    ESP_LOGI(TAG, "Initializing display subsystem");

    I2C_Init();
    LCD_Init();


    lcd_done = xSemaphoreCreateBinary();

    if (!lcd_done) {
        ESP_LOGE(TAG, "Failed to create LCD semaphore");
        return;
    }

    esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = on_color_trans_done,
    };

    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(
        io_handle, // from ST77916 driver
        &cbs, NULL));


    display_set_mode(DISPLAY_MODE_LVGL);
}

void display_set_mode(display_mode_t mode)
{
    if (current_mode == mode) {
        return;
    }

    ESP_LOGI(TAG, "Switching display mode: %d", mode);

    switch (mode) {
    case DISPLAY_MODE_LVGL:
        display_lvgl_enable();
        break;
    case DISPLAY_MODE_RAW:
        if (current_mode == DISPLAY_MODE_LVGL ||
            current_mode == DISPLAY_MODE_LVGLOverlay) {
            display_lvgl_disable();
        }
        ESP_LOGI(TAG, "Enabled raw framebuffer mode");
        break;
    case DISPLAY_MODE_LVGLOverlay:
        display_lvgl_overlay_enable();
        break;
    case DISPLAY_MODE_NULL:
        if (current_mode == DISPLAY_MODE_LVGL ||
            current_mode == DISPLAY_MODE_LVGLOverlay) {
            display_lvgl_disable();
        }
        ESP_LOGI(TAG, "Display mode set to null");
        break;
    }

    current_mode = mode;
}

void display_flush(void *buffer, size_t size) {
    if (!panel_handle) return;

  
    if (current_mode != DISPLAY_MODE_RAW) {
        return;
    }

    if (!buffer || size == 0) {
        return;
    }

    // call esp LCD driver
    // lcd_panel_draw_bitmap(buffer, 0, 0, EXAMPLE_LCD_WIDTH - 1,
    //                       EXAMPLE_LCD_HEIGHT - 1);
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, EXAMPLE_LCD_WIDTH,
                              EXAMPLE_LCD_HEIGHT, buffer);
    xSemaphoreTake(lcd_done, portMAX_DELAY);
}

