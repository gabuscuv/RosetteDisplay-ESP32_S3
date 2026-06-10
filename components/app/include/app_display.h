#pragma once
#include "display_mode_t.h"

void app_display_init(void);
void app_display_clear(void);
void app_display_flush(void);
void app_display_switch_mode(display_mode_t mode);