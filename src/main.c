#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "config.h"
#include "sensor_data.h"
#include "tasks.h"

// Define shared handles declared in sensor_data.h
QueueHandle_t xSensorQueue = NULL;
SemaphoreHandle_t xOledMutex = NULL;
EventGroupHandle_t xSystemEventGroup = NULL;

void app_main(void) {
    printf("Starting FreeRTOS Multisensor Monitoring System...\n");

    // 1. Create Synchronization & IPC Primitives
    xSensorQueue = xQueueCreate(SENSOR_QUEUE_LEN, sizeof(sensor_data_t));
    xOledMutex = xSemaphoreCreateMutex();
    xSystemEventGroup = xEventGroupCreate();

    if (xSensorQueue != NULL && xOledMutex != NULL && xSystemEventGroup != NULL) {
        // 2. Spawn FreeRTOS Tasks with designated priorities
        xTaskCreate(vSensorReadTask, "SensorReadTask", TASK_STACK_SIZE, NULL, 2, NULL);
        xTaskCreate(vDisplayTask,    "DisplayTask",    TASK_STACK_SIZE, NULL, 2, NULL);
        xTaskCreate(vAlarmTask,      "AlarmTask",      TASK_STACK_SIZE, NULL, 3, NULL);
        xTaskCreate(vInputTask,      "InputTask",      TASK_STACK_SIZE, NULL, 1, NULL);

        printf("All FreeRTOS tasks successfully created!\n");
    } else {
        printf("Error: Failed to create FreeRTOS queues/mutexes.\n");
    }
}