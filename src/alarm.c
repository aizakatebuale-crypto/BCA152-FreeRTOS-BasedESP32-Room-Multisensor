#include "alarm.h"

AlarmState evaluateTemperature(float temp) {
    if (temp < TEMP_LOW_LIMIT) return ALARM_LOW_TEMP;
    if (temp > TEMP_HIGH_LIMIT) return ALARM_HIGH_TEMP;
    return ALARM_NORMAL;
}

const char *alarmStateName(AlarmState state) {
    switch (state) {
    case ALARM_LOW_TEMP:
        return "LOW_TEMPERATURE";
    case ALARM_HIGH_TEMP:
        return "HIGH_TEMPERATURE";
    default:
        return "NORMAL";
    }
}