#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <stdbool.h>

typedef enum {
    STATE_ACTIVE,
    STATE_INACTIVE
} SystemState;

SystemState evaluateSystemState(SystemState currentState, bool motionDetected, int inactiveSeconds);

#endif