// #define FORCE_SINGLE_BUFFER
// #define BENCHMARK_MJPEG_PLAYER
#define PLAY_EVERYTHING
#ifdef PLAY_EVERYTHING
#define MAX_FILE_NAME_SIZE 32
#define MAX_FILE_NAME_COUNT 10
#endif
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

#ifdef PLAY_EVERYTHING
const char *main_directory = "videos/"; // Directory on SD card where video files are located
static char File_Name[MAX_FILE_NAME_COUNT][MAX_FILE_NAME_SIZE] = {
    "marionette.bin", "touhou.bin", "datteAnata.bin", "meltyfantasia.bin",
    "MOIW-cut3.bin", "lovely.bin", "heartprecure.bin", "gosick.bin",
    "purplegirl.bin", "parad.webm.bin"
    }; // Array to hold file names when PLAY_EVERYTHING is defined
#endif

static bool running = false;

static TaskHandle_t player_task_handle = NULL;

void (*finishedCallback)(void) = NULL;

void mjpeg_player_init(void (*cb)(void))
{
    ESP_LOGI(TAG, "Initializing MJPEG player");
    finishedCallback = cb;
}

void mjpeg_player_loop(void *arg)
{
    SD_Init();
    Flash_Searching();
#ifdef PLAY_EVERYTHING

    mjpegdecoder_init(&ctx);
    static int jpg_index = 0;

    for (int i = 0; i < MAX_FILE_NAME_COUNT && File_Name[i][0] != '\0'; i++)
    {
        ESP_LOGI(TAG, "Playing file: %s", File_Name[i]);
        char file_path[MAX_FILE_NAME_SIZE + 10];
        snprintf(file_path, sizeof(file_path), "%s%s", main_directory, File_Name[i]);
        current_url = file_path;
#endif
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s%s", mount_point, current_url);

    FILE *file = Open_File(full_path);
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file");
        vTaskDelete(NULL);
    }

    jpeg_frame_t frame;
    frame.data = ctx.jpg_buf[jpg_index];
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
          // TODO, Add logical handling for when video finishes, e.g. call a callback, switch to a different screen, etc.
          // finishedCallback();
          #ifndef PLAY_EVERYTHING
            fseek(file, 0, SEEK_SET); // rewind to the beginning of the file
            continue;
            #else
            break; // Move to the next file
            #endif
        }

    #ifdef BENCHMARK_MJPEG_PLAYER
        t1 = esp_timer_get_time();
    #endif

        // Clear the RGB buffer before decoding the next frame
        // memset(ctx.rgb_buf[ctx.buf_index], 0, SCREEN_W * SCREEN_H * 2);

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

    #ifdef FORCE_SINGLE_BUFFER
        // If using a single buffer, we need to copy the decoded frame back to the same buffer for the next iteration
        ctx.buf_index = 0;
        ctx.next = 0;
    #else
    // swap RGB buffers
    ctx.buf_index = ctx.next;
    ctx.next ^= 1;

    // swap JPEG buffers
    jpg_index ^= 1;
    frame.data = ctx.jpg_buf[jpg_index];
    #endif

    #ifdef BENCHMARK_MJPEG_PLAYER
        t4 = esp_timer_get_time();

        ESP_LOGI(TAG,
                 "read=%lldms decode=%lldms display=%lldms changeBuffer=%lldms "
                 "total=%lldms",
                 (t1 - t0) / 1000, (t2 - t1) / 1000, (t3 - t2) / 1000,
                 (t4 - t3) / 1000, (t4 - t0) / 1000);
#endif
        
    }
#ifdef PLAY_EVERYTHING
    fclose(file);
    }
#else
    fclose(file);
#endif
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