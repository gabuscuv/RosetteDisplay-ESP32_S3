#pragma once

#include <stdbool.h>

typedef enum {
  APP_MODE_NONE = -1,
  APP_MODE_SAMPLE,
  APP_MODE_VIDEO,
  APP_MODE_CONTACT,
  APP_MODE_CADIZGAMEDEV,
  APP_MODE_QR,
  APP_MODE_SLEEP  
} app_mode_t;

void app_state_init(void);

void app_state_set(app_mode_t mode);
app_mode_t app_state_get(void);

void app_state_tick(void);   // optional periodic update