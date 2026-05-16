#pragma once

#include <stdbool.h>

typedef struct {
    const int *data;
    long size;
    int width;
    int height;
} mjpeg_source_t;

void mjpeg_player_start(mjpeg_source_t src);
void mjpeg_player_stop(void);
bool mjpeg_player_is_running(void);