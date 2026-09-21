#include "alarm_logic.h"

alarm_state_t evaluate_temperature(float temperature)
{
    if (temperature < TEMP_LOW_LIMIT) {
        return ALARM_LOW_TEMPERATURE;
    }
    if (temperature > TEMP_HIGH_LIMIT) {
        return ALARM_HIGH_TEMPERATURE;
    }
    return ALARM_NORMAL;
}

const char *alarm_state_name(alarm_state_t state)
{
    switch (state) {
    case ALARM_LOW_TEMPERATURE:
        return "LOW_TEMPERATURE";
    case ALARM_HIGH_TEMPERATURE:
        return "HIGH_TEMPERATURE";
    default:
        return "NORMAL";
    }
}