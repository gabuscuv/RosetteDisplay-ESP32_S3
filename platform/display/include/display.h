#pragma once

#include <stddef.h>
#include "display_mode_t.h"

void display_init(void);
void display_set_mode(display_mode_t mode);
void display_flush(void *buffer, size_t size);