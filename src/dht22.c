#include "dht22.h"
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"

static int pulse_us(gpio_num_t pin, int level, int timeout_us)
{
    int us = 0;
    while (gpio_get_level(pin) == level) {
        if (++us > timeout_us) {
            return -1;
        }
        esp_rom_delay_us(1);
    }
    return us;
}

void dht22_init(gpio_num_t pin)
{
    gpio_reset_pin(pin);
    gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
    gpio_set_level(pin, 1);
}

esp_err_t dht22_read(gpio_num_t pin, float *temperature, float *humidity)
{
    uint8_t data[5] = {0};

    gpio_set_level(pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20));

    gpio_set_level(pin, 1);

    if (pulse_us(pin, 1, 100) < 0 ||
        pulse_us(pin, 0, 150) < 0 ||
        pulse_us(pin, 1, 150) < 0) {
        return ESP_ERR_TIMEOUT;
    }

    for (int i = 0; i < 40; i++) {
        int low = pulse_us(pin, 0, 100);
        int high = pulse_us(pin, 1, 150);
        if (low < 0 || high < 0) {
            return ESP_ERR_TIMEOUT;
        }
        data[i / 8] <<= 1;
        if (high > low) {
            data[i / 8] |= 1;
        }
    }

    if (((data[0] + data[1] + data[2] + data[3]) & 0xFF) != data[4]) {
        return ESP_ERR_INVALID_CRC;
    }

    uint16_t raw_h = ((uint16_t)data[0] << 8) | data[1];
    int16_t raw_t = (int16_t)((((uint16_t)data[2] & 0x7F) << 8) | data[3]);
    if (data[2] & 0x80) {
        raw_t = -raw_t;
    }

    *humidity = raw_h / 10.0f;
    *temperature = raw_t / 10.0f;
    return ESP_OK;
}