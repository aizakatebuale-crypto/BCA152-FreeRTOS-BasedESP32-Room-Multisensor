#ifndef ALARM_H
#define ALARM_H

#define TEMP_LOW_LIMIT   18.0f
#define TEMP_HIGH_LIMIT  30.0f

typedef enum {
    ALARM_NORMAL,
    ALARM_LOW_TEMP,
    ALARM_HIGH_TEMP
} AlarmState;

AlarmState evaluateTemperature(float temp);
const char *alarmStateName(AlarmState state);

#endif