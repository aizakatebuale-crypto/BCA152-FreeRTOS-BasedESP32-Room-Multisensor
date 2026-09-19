#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

// --- Pin Assignments ---
#define DHT_PIN         4
#define LDR_PIN         34
#define PIR_PIN         13
#define ENCODER_CLK     14
#define ENCODER_DT      27
#define ENCODER_SW      26
#define OLED_SDA        21
#define OLED_SCL        22
#define BUZZER_PIN      18

// --- Queue & Buffer Sizes ---
#define SENSOR_QUEUE_LEN 10

// --- Task Stack Sizes ---
#define TASK_STACK_SIZE 3072

// --- Event Group Bits ---
#define BIT_MOTION_DETECTED (1 << 0)
#define BIT_ALERT_TRIGGERED (1 << 1)

#endif // CONFIG_H