#include "alarm.h"

AlarmState evaluateTemperature(float temp) {
    if (temp < 18.0f) return ALARM_LOW_TEMP;
    if (temp > 30.0f) return ALARM_HIGH_TEMP;
    return ALARM_NORMAL;
}