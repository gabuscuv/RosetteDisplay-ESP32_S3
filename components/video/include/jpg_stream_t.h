#pragma once
#include "stdint.h"
#include <stddef.h>

typedef struct {
    uint8_t *data;
    size_t size;
} jpeg_frame_t;

typedef struct {
    const uint8_t *data;
    size_t size;
    size_t pos;
} jpg_stream_t;

typedef struct {
    uint8_t *jpg_buf[2];
    uint16_t *rgb_buf[2];
    volatile int buf_index;
    volatile int next;
} mjpeg_ctx_t;

typedef struct {
    jpg_stream_t stream;
    uint16_t *out_rgb;
    size_t out_stride;   // in bytes
    mjpeg_ctx_t *ctx;
} mjpeg_device_t;