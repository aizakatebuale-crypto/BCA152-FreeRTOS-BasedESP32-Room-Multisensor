typedef enum {
    ALARM_NORMAL,
    ALARM_LOW_TEMP,
    ALARM_HIGH_TEMP
} AlarmState;

AlarmState evaluateTemperature(float temp);