#ifndef TASKS_H
#define TASKS_H

#include "config.h"
#include "sensor_data.h"

void vSensorReadTask(void *pvParameters);
void vDisplayTask(void *pvParameters);
void vAlarmTask(void *pvParameters);
void vInputTask(void *pvParameters);

#endif // TASKS_H