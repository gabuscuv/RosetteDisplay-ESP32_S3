#include "mjpeg_decoder.h"
#include "tjpgd.h"
#include <driver/spi_master.h>
#include <driver/spi_common.h>
#include <driver/spi_slave.h>
#include "jpg_stream_t.h"
#include "esp_log.h"

static const char *TAG = "MJPEG_DECODER";

// #define VERBOSE_LOGGING
#define MAX_JPEG_SIZE (200 * 1024)

#define XOFF ((360 - 320) / 2)
#define YOFF ((360 - 320) / 2)

void mjpegdecoder_init(mjpeg_ctx_t *ctx) {
    ESP_LOGI(TAG, "Initializing MJPEG decoder");

    ctx->buf_index = 0;
    ctx->next = 1;
    ctx->jpg_buf[0] = heap_caps_malloc(200 * 1024, MALLOC_CAP_SPIRAM);
    ctx->jpg_buf[1] = heap_caps_malloc(200 * 1024, MALLOC_CAP_SPIRAM);

    ctx->rgb_buf[0] = heap_caps_malloc(SCREEN_W * SCREEN_H * 2, MALLOC_CAP_SPIRAM);
    assert(ctx->rgb_buf[0]);

    ctx->rgb_buf[1] = heap_caps_malloc(SCREEN_W * SCREEN_H * 2, MALLOC_CAP_SPIRAM);
    assert(ctx->rgb_buf[1]);

    ESP_LOGI(TAG, "Initialized MJPEG decoder");
}

bool mjpeg_player_next_frame(FILE *file, jpeg_frame_t *out)
{
    if (!file || !out)
    {
        ESP_LOGE(TAG, "Invalid file or output buffer");
        return false;
    }

    uint8_t buffer[2048];
    size_t bytes_read;
    size_t size = 0;
    bool in_frame = false;
    int prev = -1;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        for (size_t i = 0; i < bytes_read; i++)
        {
            uint8_t byte = buffer[i];

            if (!in_frame)
            {
                if (prev == 0xFF && byte == 0xD8)
                {
                    in_frame = true;
                    size = 0;

                    if (size + 2 > MAX_JPEG_SIZE) return false;
                    out->data[size++] = 0xFF;
                    out->data[size++] = 0xD8;
                }
            }
            else
            {
                if (size >= MAX_JPEG_SIZE)
                    return false;

                out->data[size++] = byte;

                if (prev == 0xFF && byte == 0xD9)
                {
                    out->size = size;
                    return true;
                }
            }

            prev = byte;
        }
    }

    return false;
}

// TJpgDec Callback
static int tjd_output(JDEC *jdec, void *bitmap, JRECT *rect)
{
    mjpeg_device_t *dev = (mjpeg_device_t *)jdec->device;

    uint16_t *src = (uint16_t *)bitmap;
    uint16_t *dst = dev->out_rgb;

    int w = rect->right - rect->left + 1;
    int h = rect->bottom - rect->top + 1;
    int stride_pixels = dev->out_stride / 2; // bytes → pixels

    for (int y = 0; y < h; y++) {
        memcpy(
            dst + (rect->top + y + YOFF) * stride_pixels +
                  rect->left + XOFF,
            src + y * w,
            w * 2
        );
    }
    return 1;
}

static size_t tjd_input(JDEC *jdec, uint8_t *buf, size_t len) {
    mjpeg_device_t *dev = (mjpeg_device_t *)jdec->device;
    jpg_stream_t *stream = &dev->stream;

    size_t remaining = stream->size - stream->pos;
    size_t to_read = (len < remaining) ? len : remaining;
    
    #ifdef VERBOSE_LOGGING
    ESP_LOGI(TAG,
             "Input callback: requested %d bytes, remaining %d bytes", (int)len,
             (int)remaining);
    #endif
    if (buf)
    {
        #ifdef VERBOSE_LOGGING
            ESP_LOGI(TAG, "Reading %d bytes from stream", to_read);
        #endif
        memcpy(buf, stream->data + stream->pos, to_read);
    }

    stream->pos += to_read;
    return to_read;
}

bool mjpegdecoder_decode(mjpeg_ctx_t *ctx, jpeg_frame_t *frame, uint16_t *out_rgb,
                         size_t out_stride) {
//   ESP_LOGI(TAG, "Decoding MJPEG");
    JDEC jdec;
    static uint8_t work[8192];

    mjpeg_device_t dev = {
        .stream = {
            .data = frame->data,
            .size = frame->size,
            .pos  = 0
        },
        .out_rgb   = out_rgb,
        .out_stride = out_stride,
        .ctx       = ctx
    };

#ifdef VERBOSE_LOGGING
    ESP_LOGI(TAG, "JPEG frame size = %u", (unsigned)frame->size);
#endif
    if(jd_prepare(&jdec, tjd_input, work, sizeof(work), &dev) != JDR_OK)
    {
        ESP_LOGE(TAG, "jd_prepare failed");
        return false;
    }
    if (jd_decomp(&jdec, tjd_output, 0) != JDR_OK)
    {
        ESP_LOGE(TAG, "jd_decomp failed");
        return false;
    }

    return true;
}