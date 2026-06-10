// #define BENCHMARK_MJPEG_PLAYER
#include "mjpeg_player.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "display.h"
#include "esp_log.h"
#include "SD_MMC.h"
#include "mjpeg_decoder.h"

#ifdef BENCHMARK_MJPEG_PLAYER
#include "esp_timer.h"
#endif

// #define MOCKUP

static const char *TAG = "MJPEG_PLAYER";
const char *mount_point = "/sdcard/"; // Assumes SD card is mounted at this point, adjust if needed
char *current_url = ""; // Placeholder URL for MJPEG stream

mjpeg_ctx_t ctx;

static bool running = false;

static TaskHandle_t player_task_handle = NULL;

void mjpeg_player_loop(void *arg)
{
    SD_Init();
    Flash_Searching();

    size_t len = strlen(mount_point) + strlen(current_url) + 1;
    char full_path[len];
    snprintf(full_path, len, "%s%s",
             mount_point, current_url);

    FILE *file = Open_File(full_path);
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file");
        vTaskDelete(NULL);
    }

    mjpegdecoder_init(&ctx);

    jpeg_frame_t frame;
    frame.data = ctx.jpg_buf[0];  // reuse buffer
#ifdef BENCHMARK_MJPEG_PLAYER
    int64_t t0,t1,t2,t3,t4;
#endif

    while (running)
    {
    #ifdef BENCHMARK_MJPEG_PLAYER
          t0 = esp_timer_get_time();
    #endif
        if (!mjpeg_player_next_frame(file, &frame))
        {
            ESP_LOGW(TAG, "End of stream");
            fseek(file, 0, SEEK_SET); // rewind to the beginning of the file
            continue;
        }
    #ifdef BENCHMARK_MJPEG_PLAYER
        t1 = esp_timer_get_time();
    #endif
        // Clear the RGB buffer before decoding the next frame
        memset(ctx.rgb_buf[ctx.buf_index], 0, SCREEN_W * SCREEN_H * 2);
    #ifdef BENCHMARK_MJPEG_PLAYER
        t2 = esp_timer_get_time();
    #endif
        if (mjpegdecoder_decode(&ctx, &frame, ctx.rgb_buf[ctx.buf_index], SCREEN_W * 2))
        {
            display_flush(
                ctx.rgb_buf[ctx.buf_index],
                SCREEN_W * SCREEN_H * 2
            );
        }
    #ifdef BENCHMARK_MJPEG_PLAYER
        t3 = esp_timer_get_time();
    #endif

        ctx.buf_index = ctx.next;
        ctx.next = (ctx.next + 1) % 2;

        frame.data = ctx.jpg_buf[ctx.buf_index];  // switch to the other buffer for the next frame
    #ifdef BENCHMARK_MJPEG_PLAYER
        t4 = esp_timer_get_time();

        ESP_LOGI(TAG,
                 "read=%lldms decode=%lldms display=%lldms changeBuffer=%lldms "
                 "total=%lldms",
                 (t1 - t0) / 1000, (t2 - t1) / 1000, (t3 - t2) / 1000,
                 (t4 - t3) / 1000, (t4 - t0) / 1000);
    #endif
        // vTaskDelay(pdMS_TO_TICKS(60)); // ~33 FPS
    }

    vTaskDelete(NULL);
}

void mjpeg_player_start(const char * url)
{
    ESP_LOGI(TAG, "Starting MJPEG video mode");

    current_url = url;
    running = true;

    BaseType_t result = xTaskCreate(mjpeg_player_loop, "mjpeg_player_task",
                                    4096, NULL, 5, &player_task_handle);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create MJPEG player task");
        player_task_handle = NULL;
    }
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