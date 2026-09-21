#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"

#include "config.h"
#include "sensor_data.h"
#include "tasks.h"

void vSensorReadTask(void *pvParameters) {
    sensor_data_t data;

    gpio_reset_pin(PIR_PIN);
    gpio_set_direction(PIR_PIN, GPIO_MODE_INPUT);

    gpio_reset_pin(LDR_PIN);
    gpio_set_direction(LDR_PIN, GPIO_MODE_INPUT);

    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1) {

        data.temperature = 25.5f;
        data.humidity = 60.0f;
        data.light_level = gpio_get_level(LDR_PIN) * 2000; // Simulated light reading
        data.motion_detected = (gpio_get_level(PIR_PIN) == 1);

        if (data.motion_detected) {
            xEventGroupSetBits(xSystemEventGroup, BIT_MOTION_DETECTED);
        } else {
            xEventGroupClearBits(xSystemEventGroup, BIT_MOTION_DETECTED);
        }

        xQueueSend(xSensorQueue, &data, portMAX_DELAY);

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(2000)); // Read every 2 seconds
    }
}

void vDisplayTask(void *pvParameters) {
    sensor_data_t received_data;

    while (1) {
        if (xQueueReceive(xSensorQueue, &received_data, portMAX_DELAY) == pdTRUE) {
           
            if (xSemaphoreTake(xOledMutex, portMAX_DELAY) == pdTRUE) {
                printf("\n--- ROOM MONITORING STATUS ---\n");
                printf("Temp: %.1f C | Humidity: %.1f %%\n", received_data.temperature, received_data.humidity);
                printf("Light Level: %d | Motion: %s\n", 
                       received_data.light_level, 
                       received_data.motion_detected ? "DETECTED!" : "CLEAR");
                printf("-------------------------------\n");

                xSemaphoreGive(xOledMutex);
            }
        }
    }
}

void vAlarmTask(void *pvParameters) {
    gpio_reset_pin(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    while (1) {
       
        EventBits_t bits = xEventGroupWaitBits(
            xSystemEventGroup,
            BIT_MOTION_DETECTED,
            pdFALSE, 
            pdFALSE,
            portMAX_DELAY
        );

        if (bits & BIT_MOTION_DETECTED) {
            gpio_set_level(BUZZER_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(BUZZER_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_set_level(BUZZER_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

void vInputTask(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}