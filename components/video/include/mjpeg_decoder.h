#pragma once
#include <stdio.h>
#include "jpg_stream_t.h"
#include <stdbool.h>

#define SCREEN_W 360
#define SCREEN_H 360

void mjpegdecoder_init(mjpeg_ctx_t *ctx);
bool mjpegdecoder_decode(mjpeg_ctx_t *ctx, jpeg_frame_t *frame, uint16_t *out_rgb, size_t out_stride);
bool mjpeg_player_next_frame(FILE *file, jpeg_frame_t *out);