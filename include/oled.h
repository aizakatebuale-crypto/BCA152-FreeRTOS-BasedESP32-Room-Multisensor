#ifndef OLED_H
#define OLED_H

#include "driver/gpio.h"
#include "esp_err.h"

esp_err_t oled_init(gpio_num_t sda, gpio_num_t scl);
void oled_clear(void);
void oled_draw_text(int page, int col, const char *text);
void oled_display(void);

#endif