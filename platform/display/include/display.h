#pragma once

#include <stddef.h>
typedef enum {

    DISPLAY_MODE_NULL = -1,
    DISPLAY_MODE_LVGL,
    DISPLAY_MODE_LVGLOverlay,  // future: LVGL overlay on raw framebuffer
    DISPLAY_MODE_RAW,   // future MJPEG/video mode
} display_mode_t;

void display_init(void);
void display_set_mode(display_mode_t mode);
void display_flush(void *buffer, size_t size);