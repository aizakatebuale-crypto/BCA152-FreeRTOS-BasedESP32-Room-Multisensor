#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#include "config.h"
#include "sensor_data.h"
#include "tasks.h"

// Task 1: Sensor Sampling Task (Reads simulated values and posts to Queue)
void vSensorReadTask(void *pvParameters) {
    sensor_data_t data;
    
    // Configure PIR pin as input
    gpio_reset_pin(PIR_PIN);
    gpio_set_direction(PIR_PIN, GPIO_MODE_INPUT);

    while (1) {
        // Mock readings for simulation testing
        data.temperature = 25.5f;
        data.humidity = 60.0f;
        data.light_level = adc1_get_raw(ADC1_CHANNEL_6); // Pin 34
        data.motion_detected = (gpio_get_level(PIR_PIN) == 1);

        // Signal event group if motion is detected
        if (data.motion_detected) {
            xEventGroupSetBits(xSystemEventGroup, BIT_MOTION_DETECTED);
        } else {
            xEventGroupClearBits(xSystemEventGroup, BIT_MOTION_DETECTED);
        }

        // Post data to queue
        xQueueSend(xSensorQueue, &data, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(2000)); // Read every 2 seconds
    }
}

// Task 2: Display Task (Consumes Queue data and updates status safely)
void vDisplayTask(void *pvParameters) {
    sensor_data_t received_data;

    while (1) {
        if (xQueueReceive(xSensorQueue, &received_data, portMAX_DELAY) == pdTRUE) {
            // Protect display resource with Mutex
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

// Task 3: Alarm Task (Waits for Event Group bit to sound buzzer)
void vAlarmTask(void *pvParameters) {
    gpio_reset_pin(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    while (1) {
        // Wait for motion event bit
        EventBits_t bits = xEventGroupWaitBits(
            xSystemEventGroup,
            BIT_MOTION_DETECTED,
            pdFALSE, // Do not clear on exit
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

// Task 4: Input / Control Task
void vInputTask(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}