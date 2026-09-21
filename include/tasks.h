#ifndef TASKS_H
#define TASKS_H

void vSensorReadTask(void *pvParameters);
void vDisplayTask(void *pvParameters);
void vAlarmTask(void *pvParameters);
void vInputTask(void *pvParameters);
void vMotionTask(void *pvParameters);
void vStateTask(void *pvParameters);

#endif