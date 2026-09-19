#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

typedef struct {
    float temperature;
    float humidity;
    int light_level;
    bool motion_detected;
} sensor_data_t;

// Shared FreeRTOS Handles
extern QueueHandle_t xSensorQueue;
extern SemaphoreHandle_t xOledMutex;
extern EventGroupHandle_t xSystemEventGroup;

#endif // SENSOR_DATA_H