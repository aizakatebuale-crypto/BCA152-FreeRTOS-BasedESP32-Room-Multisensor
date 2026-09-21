#ifndef ALARM_LOGIC_H
#define ALARM_LOGIC_H

#define TEMP_LOW_LIMIT   18.0f
#define TEMP_HIGH_LIMIT  30.0f

typedef enum {
    ALARM_NORMAL,
    ALARM_LOW_TEMPERATURE,
    ALARM_HIGH_TEMPERATURE
} alarm_state_t;

alarm_state_t evaluate_temperature(float temperature);
const char *alarm_state_name(alarm_state_t state);

#endif