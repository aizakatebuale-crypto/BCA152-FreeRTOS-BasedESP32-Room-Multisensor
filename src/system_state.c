#include "system_state.h"

SystemState evaluateSystemState(SystemState currentState, bool motionDetected, int inactiveSeconds) {

    if (motionDetected) {
        return STATE_ACTIVE;
    }

    if (currentState == STATE_ACTIVE && inactiveSeconds >= INACTIVITY_TIMEOUT_SECONDS) {
        return STATE_INACTIVE;
    }

    return currentState;
}