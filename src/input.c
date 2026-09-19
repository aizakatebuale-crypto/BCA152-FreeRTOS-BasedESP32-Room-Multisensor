#include "input.h"

DisplayMode nextDisplayMode(DisplayMode current) {
    if (current >= MODE_MOTION) {
        return MODE_TEMP; // Wraparound to first screen
    }
    return (DisplayMode)(current + 1);
}

DisplayMode previousDisplayMode(DisplayMode current) {
    if (current <= MODE_TEMP) {
        return MODE_MOTION; // Wraparound to last screen
    }
    return (DisplayMode)(current - 1);
}