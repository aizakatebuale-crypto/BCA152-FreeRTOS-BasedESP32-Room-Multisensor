#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

typedef struct {
    float temperature;
    float humidity;
    int light_level;
    bool motion_detected;
} sensor_data_t;

extern QueueHandle_t xSensorQueue;
extern QueueHandle_t xAlarmQueue;
extern SemaphoreHandle_t xSerialMutex;
extern EventGroupHandle_t xSystemEventGroup;

#endif