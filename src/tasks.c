#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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
#include "alarm.h"
#include "system_state.h"
#include "input.h"
#include "oled.h"

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

static const char *display_mode_name(DisplayMode mode)
{
    switch (mode) {
    case MODE_TEMP:      return "TEMPERATURE";
    case MODE_HUMIDITY:  return "HUMIDITY";
    case MODE_LIGHT:     return "LIGHT";
    case MODE_MOTION:    return "MOTION";
    default:             return "UNKNOWN";
    }
}

void vSensorReadTask(void *pvParameters) {
    sensor_data_t data;

    dht22_init(DHT_PIN);
    ldr_init();

    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1) {
        float temperature, humidity;
        esp_err_t err = dht22_read(DHT_PIN, &temperature, &humidity);

        if (err == ESP_OK) {
            EventBits_t bits = xEventGroupGetBits(xSystemEventGroup);

            data.temperature = temperature;
            data.humidity = humidity;
            data.light_level = ldr_read_percent();
            data.motion_detected = (bits & BIT_MOTION_DETECTED) != 0;

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
    char line1[20], line2[20], line3[20];

    oled_init(OLED_SDA, OLED_SCL);

    while (1) {
        if (xQueueReceive(xSensorQueue, &received_data, portMAX_DELAY) == pdTRUE) {
            EventBits_t bits = xEventGroupGetBits(xSystemEventGroup);

            if ((bits & BIT_SYSTEM_ACTIVE) == 0) {
                continue;
            }

            snprintf(line1, sizeof(line1), "T:%.1fC H:%.1f%%", received_data.temperature, received_data.humidity);
            snprintf(line2, sizeof(line2), "Light: %d%%", received_data.light_level);
            snprintf(line3, sizeof(line3), "Motion: %s", received_data.motion_detected ? "YES" : "NO");

            oled_clear();
            oled_draw_text(0, 0, line1);
            oled_draw_text(2, 0, line2);
            oled_draw_text(4, 0, line3);
            oled_display();

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
    AlarmState last_state = ALARM_NORMAL;

    buzzer_init();

    while (1) {
        if (xQueueReceive(xAlarmQueue, &data, portMAX_DELAY) == pdTRUE) {
            AlarmState state = evaluateTemperature(data.temperature);
            EventBits_t bits = xEventGroupGetBits(xSystemEventGroup);
            bool system_active = (bits & BIT_SYSTEM_ACTIVE) != 0;

            if (state != ALARM_NORMAL) {
                xEventGroupSetBits(xSystemEventGroup, BIT_ALERT_TRIGGERED);
                buzzer_set(system_active);
            } else {
                xEventGroupClearBits(xSystemEventGroup, BIT_ALERT_TRIGGERED);
                buzzer_set(false);
            }

            if (state != last_state) {
                if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
                    printf("ALARM state: %s (%.1f C)\n", alarmStateName(state), data.temperature);
                    xSemaphoreGive(xSerialMutex);
                }
                last_state = state;
            }
        }
    }
}

void vMotionTask(void *pvParameters) {
    gpio_reset_pin(PIR_PIN);
    gpio_set_direction(PIR_PIN, GPIO_MODE_INPUT);

    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1) {
        if (gpio_get_level(PIR_PIN) == 1) {
            xEventGroupSetBits(xSystemEventGroup, BIT_MOTION_DETECTED);
        } else {
            xEventGroupClearBits(xSystemEventGroup, BIT_MOTION_DETECTED);
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(MOTION_POLL_MS));
    }
}

void vStateTask(void *pvParameters) {
    SystemState state = STATE_ACTIVE;
    int inactive_ms = 0;

    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1) {
        EventBits_t bits = xEventGroupGetBits(xSystemEventGroup);
        bool motion = (bits & BIT_MOTION_DETECTED) != 0;

        if (motion) {
            inactive_ms = 0;
        } else if (inactive_ms < INACTIVITY_TIMEOUT_SECONDS * 1000) {
            inactive_ms += STATE_CHECK_MS;
        }

        SystemState next = evaluateSystemState(state, motion, inactive_ms / 1000);

        if (next != state) {
            if (next == STATE_ACTIVE) {
                xEventGroupSetBits(xSystemEventGroup, BIT_SYSTEM_ACTIVE);
            } else {
                xEventGroupClearBits(xSystemEventGroup, BIT_SYSTEM_ACTIVE);
            }

            if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
                printf("SYSTEM state: %s\n", next == STATE_ACTIVE ? "ACTIVE" : "INACTIVE");
                xSemaphoreGive(xSerialMutex);
            }

            state = next;
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(STATE_CHECK_MS));
    }
}

void vInputTask(void *pvParameters) {
    gpio_reset_pin(ENCODER_CLK);
    gpio_set_direction(ENCODER_CLK, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ENCODER_CLK, GPIO_PULLUP_ONLY);

    gpio_reset_pin(ENCODER_DT);
    gpio_set_direction(ENCODER_DT, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ENCODER_DT, GPIO_PULLUP_ONLY);

    int lastClkState = gpio_get_level(ENCODER_CLK);

    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1) {
        EventBits_t bits = xEventGroupGetBits(xSystemEventGroup);
        bool system_active = (bits & BIT_SYSTEM_ACTIVE) != 0;

        if (system_active) {
            int clkState = gpio_get_level(ENCODER_CLK);

            if (clkState != lastClkState && clkState == 0) {
                int dtState = gpio_get_level(ENCODER_DT);


                if (xSemaphoreTake(xDisplayModeMutex, portMAX_DELAY) == pdTRUE) {
                    DisplayMode newMode;
                    if (dtState != clkState) {
                        newMode = nextDisplayMode(currentDisplayMode);
                    } else {
                        newMode = previousDisplayMode(currentDisplayMode);
                    }
                    currentDisplayMode = newMode;
                    xSemaphoreGive(xDisplayModeMutex);

                    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
                        printf("Display mode: %s\n", display_mode_name(newMode));
                        xSemaphoreGive(xSerialMutex);
                    }
                }
            }

            lastClkState = clkState;
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(ENCODER_POLL_MS));
    }
}