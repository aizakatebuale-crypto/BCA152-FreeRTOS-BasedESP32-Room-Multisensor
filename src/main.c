#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "config.h"
#include "sensor_data.h"
#include "tasks.h"

QueueHandle_t xSensorQueue = NULL;
QueueHandle_t xAlarmQueue = NULL;
SemaphoreHandle_t xSerialMutex = NULL;
EventGroupHandle_t xSystemEventGroup = NULL;

volatile DisplayMode currentDisplayMode = MODE_TEMP;
SemaphoreHandle_t xDisplayModeMutex = NULL;

void app_main(void) {
    printf("Starting FreeRTOS Multisensor Monitoring System...\n");

    xSensorQueue = xQueueCreate(SENSOR_QUEUE_LEN, sizeof(sensor_data_t));
    xAlarmQueue = xQueueCreate(SENSOR_QUEUE_LEN, sizeof(sensor_data_t));
    xSerialMutex = xSemaphoreCreateMutex();
    xSystemEventGroup = xEventGroupCreate();
    xDisplayModeMutex = xSemaphoreCreateMutex();

    if (xSensorQueue != NULL && xAlarmQueue != NULL &&
        xSerialMutex != NULL && xSystemEventGroup != NULL &&
        xDisplayModeMutex != NULL) {

        xEventGroupSetBits(xSystemEventGroup, BIT_SYSTEM_ACTIVE);

        xTaskCreate(vSensorReadTask, "SensorReadTask", TASK_STACK_SIZE, NULL, 2, NULL);
        xTaskCreate(vDisplayTask,    "DisplayTask",    TASK_STACK_SIZE, NULL, 1, NULL);
        xTaskCreate(vAlarmTask,      "AlarmTask",      TASK_STACK_SIZE, NULL, 2, NULL);
        xTaskCreate(vInputTask,      "InputTask",      TASK_STACK_SIZE, NULL, 3, NULL);
        xTaskCreate(vMotionTask,     "MotionTask",     TASK_STACK_SIZE, NULL, 3, NULL);
        xTaskCreate(vStateTask,      "StateTask",      TASK_STACK_SIZE, NULL, 2, NULL);

        if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
            printf("All FreeRTOS tasks successfully created!\n");
            xSemaphoreGive(xSerialMutex);
        }
    } else {
        printf("Error: Failed to create FreeRTOS queues/mutexes.\n");
    }
}