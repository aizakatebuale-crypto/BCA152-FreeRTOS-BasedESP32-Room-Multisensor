#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

#include "config.h"
#include "sensor_data.h"
#include "tasks.h"
#include "dht22.h"
#include "ldr.h"
#include "alarm_logic.h"

static void buzzer_init(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_cfg);

    ledc_channel_config_t channel_cfg = {
        .gpio_num = BUZZER_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&channel_cfg);
}

static void buzzer_set(bool on)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, on ? 512 : 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void vSensorReadTask(void *pvParameters) {
    sensor_data_t data;

    dht22_init(DHT_PIN);
    ldr_init();

    gpio_reset_pin(PIR_PIN);
    gpio_set_direction(PIR_PIN, GPIO_MODE_INPUT);

    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1) {
        float temperature, humidity;
        esp_err_t err = dht22_read(DHT_PIN, &temperature, &humidity);

        if (err == ESP_OK) {
            data.temperature = temperature;
            data.humidity = humidity;
            data.light_level = ldr_read_percent();
            data.motion_detected = (gpio_get_level(PIR_PIN) == 1);

            if (data.motion_detected) {
                xEventGroupSetBits(xSystemEventGroup, BIT_MOTION_DETECTED);
            } else {
                xEventGroupClearBits(xSystemEventGroup, BIT_MOTION_DETECTED);
            }

            xQueueSend(xSensorQueue, &data, pdMS_TO_TICKS(100));
            xQueueSend(xAlarmQueue, &data, pdMS_TO_TICKS(100));
        } else {
            if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
                printf("DHT22 read failed: %s\n", esp_err_to_name(err));
                xSemaphoreGive(xSerialMutex);
            }
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(2000));
    }
}

void vDisplayTask(void *pvParameters) {
    sensor_data_t received_data;

    while (1) {
        if (xQueueReceive(xSensorQueue, &received_data, portMAX_DELAY) == pdTRUE) {

            if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
                printf("\n--- ROOM MONITORING STATUS ---\n");
                printf("Temp: %.1f C | Humidity: %.1f %%\n", received_data.temperature, received_data.humidity);
                printf("Light Level: %d | Motion: %s\n",
                       received_data.light_level,
                       received_data.motion_detected ? "DETECTED!" : "CLEAR");
                printf("-------------------------------\n");

                xSemaphoreGive(xSerialMutex);
            }
        }
    }
}

void vAlarmTask(void *pvParameters) {
    sensor_data_t data;
    alarm_state_t last_state = ALARM_NORMAL;

    buzzer_init();

    while (1) {
        if (xQueueReceive(xAlarmQueue, &data, portMAX_DELAY) == pdTRUE) {
            alarm_state_t state = evaluate_temperature(data.temperature);

            if (state != ALARM_NORMAL) {
                xEventGroupSetBits(xSystemEventGroup, BIT_ALERT_TRIGGERED);
                buzzer_set(true);
            } else {
                xEventGroupClearBits(xSystemEventGroup, BIT_ALERT_TRIGGERED);
                buzzer_set(false);
            }

            if (state != last_state) {
                if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
                    printf("ALARM state: %s (%.1f C)\n", alarm_state_name(state), data.temperature);
                    xSemaphoreGive(xSerialMutex);
                }
                last_state = state;
            }
        }
    }
}

void vInputTask(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}