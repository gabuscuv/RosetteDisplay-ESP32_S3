#pragma once

#include <stdbool.h>

void mjpeg_player_init(void (*cb)(void));
void mjpeg_player_start(const char * url);
void mjpeg_player_stop(void);
bool mjpeg_player_is_running(void);