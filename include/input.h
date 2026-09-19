#ifndef INPUT_H
#define INPUT_H

typedef enum {
    MODE_TEMP,
    MODE_HUMIDITY,
    MODE_LIGHT,
    MODE_MOTION,
    MODE_COUNT
} DisplayMode;

DisplayMode nextDisplayMode(DisplayMode current);
DisplayMode previousDisplayMode(DisplayMode current);

#endif