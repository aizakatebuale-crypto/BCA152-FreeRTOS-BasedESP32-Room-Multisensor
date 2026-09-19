#include "system_state.h"

SystemState evaluateSystemState(SystemState currentState, bool motionDetected, int inactiveSeconds) {

    if (motionDetected) {
        return STATE_ACTIVE;
    }

    if (currentState == STATE_ACTIVE && inactiveSeconds >= 15) {
        return STATE_INACTIVE;
    }

    return currentState;
}