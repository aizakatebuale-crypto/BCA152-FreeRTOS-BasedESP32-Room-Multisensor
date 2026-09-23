# BCA152 FreeRTOS-Based ESP32 ROOM Multisensor

This project is an ESP32-based room monitoring system that uses ESP-IDF and FreeRTOS. Temperature, humidity, light, and motion are all measured, and the results are displayed one at a time on an OLED screen. Additionally, it has a buzzer that sounds when the temperature is too high or too low.

## Project Overview

For our BCA152 Laboratory 1, this project was created. The objective is to use ESP-IDF and FreeRTOS to create a real-time multisensor system from the ground up. Since the lab requires the native FreeRTOS APIs, we did not use Arduino.

SensorReadTask, DisplayTask, AlarmTask, InputTask, MotionTask, and StateTask are the six FreeRTOS tasks in the system. They communicate with one another through event groups, mutexes, and queues. Since we don't have a physical ESP32, the entire system is simulated in Wokwi.

This lab's primary focus is on concurrent embedded system design. This entails understanding how to prioritize tasks, how blocking operates, and how to safeguard shared data to prevent conflicts between tasks.

## Features

- Measures temperature and humidity using a DHT22 sensor
- Measures ambient light using a LDR (photoresistor) through the ESP ADC
- Detects motion using a PIR sensor
- Shows one measurement at a time on an SSD1306 OLED display
- Lets the user switch between Temperature, Humidity, Light, and Motion using a rotary encoder
- Turns on a buzzer when the temperature goes below 18°C or above 30°C
- Supports two system states: ACTIVE and INACTIVE
- Automatically goes INACTIVE after 15 seconds without motion
- Automatically goes back to ACTIVE when motion is being detected again

## Learning Objectives

- Create an ESP32 project using PlatformIO
- Build and simulate a circuit in Wokwi
- Connect actuators and sensors to an ESP32
- Organize the firmware into several source files
- Create FreeRTOS tasks
- Recognize the distinctions between the task states of Running, Ready, Blocked, Suspended, and Deleted
- Assign and describe the order of importance of each task
- Use queues to facilitate communication between tasks
- To protect a shared resource, use a mutex
- For signaling, use a task notification or event group.
- For recurring tasks, use `vTaskDelayUntil()`
- Construct a basic state machine
- Keep hardware drivers and decision logic apart
- Use PlatformIO to create and execute unit tests
- Maintain a GitHub repository and use Git
- Document a project for both school and portfolio use
- Share the project on Hackster.io

## System Architecture

```
INPUTS (Sensors)          PROCESSING           OUTPUTS
                                                 
+-----------+                                    
|  DHT22    |  GPIO 4      +-----------+     +-----------+
| (temp &   |------------->|           |---->|  OLED     |
| humidity) |              |           |     | SSD1306   |
+-----------+              |           |     | (SDA 21,  |
                           |   ESP32   |     |  SCL 22)  |
+-----------+              |           |     +-----------+
|   LDR     |  GPIO 34     | ESP-IDF + |     
| (light)   |------------->| FreeRTOS  |     +-----------+
+-----------+              |           |---->|  Buzzer   |
                           |           |     |  GPIO 18  |
+-----------+              |           |     +-----------+
|   PIR     |  GPIO 13     |           |     
| (motion)  |------------->|           |     +-----------+
+-----------+              |           |---->| Encoder   |
                           |           |     | CLK 14,   |
+-----------+              |           |     | DT 27     |
| Encoder   |  GPIO 14,27  |           |     +-----------+
| (user     |------------->|           |     
|  input)   |              +-----------+     +-----------+
                           |           |---->| Serial    |
                           +-----------+     | Monitor   |
                                             +-----------+
```

The diagram illustrates how the ESP32 is connected to the sensors and output devices. The ESP32 receives data from the DHT22, LDR, and PIR. After processing the data, the ESP32 outputs it to the serial monitor, buzzer, encoder, and OLED

## FreeRTOS Architecture

Six FreeRTOS tasks are used by the system. Every task has a distinct role, a priority, and a communication channel with other tasks.

**Tasks and Priorities:** 

- **MotionTask (Priority 3)** — reads the PIR sensor every 100 ms. Sets the bit of the `BIT_MOTION_DETECTED` event when the movement is detected. The highest priority is because the movement must be detected quickly. 
- **InputTask (Priority 3)** — reads the rotary encoder every 10 ms. Update the shared `currentDisplayMode` variable (protected by xDisplayModeMutex). Highest priority because the user's input must feel responsive.
 - **SensorReadTask (Priority 2)** - Reads DHT22 and LDR every 2 seconds using `vTaskDelayUntil()`. Send the data to `xSensorQueue` and `xAlarmQueue`. 
- **AlarmTask (Priority 2)** — wait for `xAlarmQueue`. Check the temperature and turn on or off the buzzer. Set or delete the bit of the `BIT_ALERT_TRIGGERED` event. 
- **StateTask (Priority 2)** — checks the state of the system every 100 ms. If no movement is detected for 15 seconds, it turns the system into INACTIVE. Sets or clears the `BIT_SYSTEM_ACTIVE` event bit.
 - **DisplayTask (priority 1)** — wait for `xSensorQueue`. Update the OLED with current measurement. Low priority because display updates are not urgent. 

 **Data flow between tasks:**

 ```
 SensorReadTask --[xSensorQueue]--> DisplayTask
                \--[xAlarmQueue] ---> AlarmTask

MotionTask -----[Event: BIT_MOTION_DETECTED]--> StateTask

StateTask ------[Event: BIT_SYSTEM_ACTIVE]----> DisplayTask, InputTask, AlarmTask

AlarmTask ------[Event: BIT_ALERT_TRIGGERED]--> (system-wide signal)

InputTask ------[Mutex: xDisplayModeMutex]----> DisplayTask (shares currentDisplayMode)

All tasks ------[Mutex: xSerialMutex]---------> Serial monitor (shared printf)
```

**Synchronization summary:**

- **Queues (`xSensorQueue`, `xAlarmQueue`)** — pass `sensor_data_t` structs from SensorReadTask to DisplayTask and AlarmTask.
- **Mutexes (`xSerialMutex`, `xDisplayModeMutex`)** — protect shared data (`printf` output and `currentDisplayMode`).
- **Event group (`xSystemEventGroup`)** — signals state changes across tasks.

## Hardware / Simulated Components

All components are simulated in Wokwi

| Component | Purpose |
|-----------|---------|
| ESP32 DevKit | Main microcontroller |
| DHT22 | Temperature and humidity sensor |
| Photoresistor (LDR) | Ambient light sensor |
| PIR sensor | Motion detection |
| Rotary encoder | User input for navigation |
| SSD1306 OLED | Information display (I²C) |
| Buzzer | Alarm output |

## Pin Configuration

| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| DHT22 DATA | GPIO 4 | Digital input |
| LDR (ADC) | GPIO 34 | ADC1 channel 6 (input-only pin) |
| PIR OUT | GPIO 13 | Digital input |
| Encoder CLK | GPIO 14 | Digital input, pull-up enabled |
| Encoder DT | GPIO 27 | Digital input, pull-up enabled |
| Encoder SW | GPIO 26 | Reserved for future use |
| OLED SDA | GPIO 21 | I²C data line |
| OLED SCL | GPIO 22 | I²C clock line |
| Buzzer | GPIO 18 | LEDC PWM output |

## Task Design

| Task | Priority | Period / Trigger | IPC | Blocked Condition |
|------|----------|------------------|-----|-------------------|
| MotionTask | 3 | 100 ms (`vTaskDelayUntil`) | Event group | `vTaskDelayUntil` |
| InputTask | 3 | 10 ms (`vTaskDelayUntil`) | Mutex (`xDisplayModeMutex`) | `vTaskDelayUntil` |
| SensorReadTask | 2 | 2000 ms (`vTaskDelayUntil`) | Queue (`xSensorQueue`, `xAlarmQueue`) | `vTaskDelayUntil` |
| AlarmTask | 2 | Queue-driven | Queue (`xAlarmQueue`) | `xQueueReceive` |
| StateTask | 2 | 100 ms (`vTaskDelayUntil`) | Event group | `vTaskDelayUntil` |
| DisplayTask | 1 | Queue-driven | Queue (`xSensorQueue`) | `xQueueReceive` |

**Why these priorities?**

- **MotionTask and InputTask(Priority3)** — These require the fastest response. If themotion detection or encoder input is delayed, the system feels slow. Motion must be detected before the inactivity timer runs out. 
- **SensorReadTask,AlarmTaskand StateTask(Priority2)** — These workin regular time with limited latency. The two-second sensor cycle gives them space, but alarms still have to react before the next cycle.
 - **DisplayTask(Priority1)** — Display updates are the least urgent. A small delay before the OLED is refreshed does not affect the system behaviour, and the task is executed only when new data arrives from the queue.

 ## Inter-Task Communication

The system uses three types of FreeRTOS synchronization objects: queues, mutexes, and an event group

 **Queues:** 
 
 -**xSensorQueue** — Transports the `sensor_data_t` structure from the SensorReadTask to the DisplayTask. Length: 10 items. 

 -**xAlarmQueue** — Transports `sensor_data_t` structs from SensorReadTask to AlarmTask. Length: 10 items.
 
 **Mutexes:**  

 -**xSerialMutex**— Protects the calls of `printf()`. All tasks must use this mutex before printing to avoid interleaved output. 
 
 -**xDisplayModeMutex** — Protects the common `currentDisplayMode` variable. InputTask writes to it; DisplayTask reads it. 
 
 **EventGroup:** 
 
 -**xSystemEventGroup** - Signaling system-wide events between tasks: 
 - `BIT_MOTION_DETECTED` (bit 0) - Set by MotionTask when the PIR detects movement. 
 Read by StateTask and SensorReadTask. - `BIT_ALERT_TRIGGERED` (bit 1) - Set by AlarmTask when the temperature is out of range. Read for any task that needs to know about the alarm. 
 - `BIT_SYSTEM_ACTIVE` (bit 2) – Set or remove by StateTask. Read DisplayTask, InputTask, and AlarmTask to know if the system is ACTIVE or INACTIVE.

 **Why use a queue instead of global variables?**

Unsynchronized global variables can be read while another task is writing to them, causing torn or inconsistent data. A queue handles this automatically: the sender blocks if the queue is full, and the receiver blocks if the queue is empty. This makes the data flow safe and predictable.

## State Machine

The system has two states: **ACTIVE** and **INACTIVE**

- **ACTIVE** — Normal operation. The OLED is on, sensor processing runs, encoder navigation works, and the alarm is active.
- **INACTIVE** — Low-power mode. The OLED is blank, sensor display operations are reduced, the buzzer is off, but motion detection still runs so the system can wake up.

**State transitions:**

```
             motion detected
   INACTIVE -------------------> ACTIVE
      ^                            |
      |                            |
      +----------------------------+
         no motion for 15 seconds
```

The state is managed by **StateTask**, which runs every 100 ms. It reads the `BIT_MOTION_DETECTED` event bit and keeps a timer of how long the system has been without motion. When the timer reaches 15 seconds, StateTask switches to INACTIVE and clears the `BIT_SYSTEM_ACTIVE` event bit. When motion is detected again, StateTask switches back to ACTIVE and sets the bit.

The decision logic is separated into a pure function `evaluateSystemState()` in `system_state.c`. This makes it testable without any hardware.

## Repository Structure

```
BCA152-freeRTOS-multisensor/
├── include/
│   ├── alarm.h
│   ├── config.h
│   ├── dht22.h
│   ├── font5x7.h
│   ├── input.h
│   ├── ldr.h
│   ├── oled.h
│   ├── sensor_data.h
│   ├── system_state.h
│   └── tasks.h
├── src/
│   ├── CMakeLists.txt
│   ├── alarm.c
│   ├── dht22.c
│   ├── input.c
│   ├── ldr.c
│   ├── main.c
│   ├── oled.c
│   ├── system_state.c
│   └── tasks.c
├── test/
│   ├── README
│   └── test_main.c
├── docs/
│   ├── build-success.png
│   ├── fault1-remove-blocking.png
│   ├── fault2-high-priority.png
│   ├── fault3-no-mutex-before.png
│   ├── fault3-no-mutex-after.png
│   ├── ft01-temp-before.png
│   ├── ft01-temp-after.png
│   ├── ft02-humidity-before.png
│   ├── ft02-humidity-after.png
│   ├── ft03-light-low.png
│   ├── ft03-light-high.png
│   ├── ft04-encoder-navigation-verified.png
│   ├── ft05-encoder-ccw.png
│   ├── ft06-alarm-normal.png
│   ├── ft07-alarm-high.png
│   ├── ft08-motion-active.png
│   ├── ft09-inactive-timeout.png
│   ├── ft10-motion-reactivation.png
│   ├── git-log.png
│   ├── oled-display.png
│   ├── unit-tests-pass.png
│   ├── unit-tests-fail-demo.png
│   └── wokwi-circuit.png
├── .gitignore
├── CMakeLists.txt
├── diagram.json
├── platformio.ini
├── sdkconfig.esp32dev
├── wokwi.toml
└── README.md
```

## Getting Started

### Prerequisites

To build and run this project, you need:

- **Visual Studio Code** with the **PlatformIO IDE** extension installed.
- **Python 3.8 or newer** (required by PlatformIO).
- **Git** for cloning the repository.
- **Wokwi for VS Code** extension (for running the simulation).

### Clone the Repository

Open a terminal and run:

```bash
git clone https://github.com/aizakatebuale-crypto/BCA152-FreeRTOS-BasedESP32-Room-Multisensor.git
cd BCA152-FreeRTOS-BasedESP32-Room-Multisensor
```

Then open the folder in VS Code:

```bash
code .
```

## Building the Project

To build the firmware for the ESP32:

```bash
pio run -e esp32dev
```

A successful build should end with `[SUCCESS]`. The expected resource usage is:

- **Flash:** approximately 17.7% (around 186 KB used out of 1 MB)
- **RAM:** approximately 4.6% (around 15 KB used out of 320 KB)

To clean the build files:

```bash
pio run -e esp32dev -t clean
```

## Running the Wokwi Simulation

This project is simulated in Wokwi. The circuit is defined in `diagram.json`

To run the simulation:

1. Open the project folder in VS Code.
2. Install the **Wokwi for VS Code** extension if you have not already.
3. Press `F1` and run the command **Wokwi: Start Simulator**.
4. The simulation will start and the OLED should display the current temperature.
5. Use the Wokwi controls to rotate the encoder, change the temperature, or trigger the PIR sensor.

**What to expect:**

- The OLED shows `ROOM MONITOR` on the first line.
- The second line shows the current display mode (Temperature, Humidity, Light, or Motion).
- The third line shows the value of the selected measurement.
- Rotating the encoder switches between pages.
- When temperature goes below 18 °C or above 30 °C, the buzzer sounds.
- After 15 seconds without motion, the OLED goes blank (INACTIVE state).
- Triggering the PIR sensor brings the system back to ACTIVE.

## Unit Testing

The hardware-independent logic is tested using PlatformIO's native test environment. This means the tests run on the PC, not on the ESP32.

To run the tests:

```bash
pio test -e native
```

**Result:** 13 out of 13 tests pass.

**Test coverage:**

**Temperature alarm logic (5 tests):**
- Below lower threshold (< 18 °C)
- Exactly at lower threshold (= 18 °C)
- Normal value (18 °C < temp < 30 °C)
- Exactly at upper threshold (= 30 °C)
- Above upper threshold (> 30 °C)

**Display navigation (4 tests):**
- Forward transition (Temperature → Humidity → Light → Motion)
- Forward wraparound (Motion → Temperature)
- Reverse transition (Motion → Light → Humidity → Temperature)
- Reverse wraparound (Temperature → Motion)

**System state (4 tests):**
- ACTIVE without timeout
- ACTIVE with timeout (switches to INACTIVE)
- INACTIVE without motion (stays INACTIVE)
- INACTIVE with motion (switches to ACTIVE)

See `docs/unit-tests-pass.png` for the screenshot of the test results.

## Static Code Analysis

Static code analysis is done using `cppcheck` through PlatformIO.

To run it:

```bash
pio check -e esp32dev
```

**Result:** PASSED — 0 HIGH, 0 MEDIUM, 20 LOW.

**Findings and resolutions:**

| Finding | File / Line | Cause | Resolution |
|---------|-------------|-------|------------|
| `unusedFunction` (×20) | `alarm.c`, `dht22.c`, `input.c`, `ldr.c`, `main.c`, `oled.c`, `system_state.c`, `tasks.c` | `cppcheck` analyzes each `.c` file separately and cannot see cross-file calls. Each function is used in another file. | Accepted as a false positive; documented. |
| `constVariable` | `src/oled.c:17` | The buffer `buf` was not modified after initialization. | Declared as `const uint8_t buf[2]`. |
| `variableScope` | `src/tasks.c:260` | The variable `newMode` was declared in the outer block but only used inside an `if` block. | Moved the declaration inside the `if` block. |

The two real findings (`constVariable` and `variableScope`) were fixed. The remaining `unusedFunction` warnings are a known limitation of `cppcheck` when working with multi-file C projects.

## Functional Verification

Each functional requirement was tested in the Wokwi simulation. Screenshots of the actual results are stored in the `docs/` folder.

| Test ID | Stimulus | Expected | Actual | Result |
|---------|----------|----------|--------|--------|
| FT-01 | Change temperature | Displayed temperature updates | OLED showed new temperature | ✅ PASS |
| FT-02 | Change humidity | Displayed humidity updates | OLED showed new humidity | ✅ PASS |
| FT-03 | Change light input | Light value changes | OLED showed new light value | ✅ PASS |
| FT-04 | Rotate encoder clockwise | Next page selected | Moved to next mode | ✅ PASS |
| FT-05 | Rotate encoder counterclockwise | Previous page selected | Moved to previous mode | ✅ PASS |
| FT-06 | Set temperature above 30 °C | Alarm activates | Buzzer turned on | ✅ PASS |
| FT-07 | Return temperature to normal | Alarm stops | Buzzer turned off | ✅ PASS |
| FT-08 | Trigger PIR | System is ACTIVE | System stayed ACTIVE | ✅ PASS |
| FT-09 | Allow inactivity timeout (15 s) | System becomes INACTIVE | OLED went blank, system INACTIVE | ✅ PASS |
| FT-10 | Trigger PIR while INACTIVE | System returns to ACTIVE | OLED came back, system ACTIVE | ✅ PASS |

All 10 functional tests passed.

## Engineering Decisions

- **Using ESP-IDF instead of Arduino** — The laboratory requires the native FreeRTOS APIs, so Arduino was not allowed. ESP-IDF gives direct access to FreeRTOS tasks, queues, mutexes, event groups, and the LEDC PWM peripheral.
- **One task per responsibility** — Each task does one job. SensorReadTask handles sensors, DisplayTask handles the OLED, AlarmTask handles the buzzer, and so on. This makes the code easier to test and reason about.
- **Queues instead of global variables for sensor data** — Sensor data flows from SensorReadTask to DisplayTask and AlarmTask through queues. This avoids unsynchronized access and gives automatic blocking behavior when the queue is full or empty.
- **`vTaskDelayUntil()` instead of `vTaskDelay()` for periodic tasks** — `vTaskDelayUntil()` anchors the next wake time to a fixed tick count, so the period does not drift even if a task occasionally takes longer than expected. `vTaskDelay()` would add the task's own execution time to the delay and slowly drift.
- **Event group instead of global flags** — Event bits are set and cleared atomically, so tasks always see a consistent value. This avoids the race conditions that can occur with plain `bool` flags.
- **Two mutexes** — One mutex (`xSerialMutex`) protects the shared serial output, and another (`xDisplayModeMutex`) protects the shared `currentDisplayMode` variable. Each mutex has a clear, specific purpose.
- **Separating decision logic from hardware drivers** — Functions like `evaluateTemperature()`, `nextDisplayMode()`, and `evaluateSystemState()` are pure functions with no hardware dependencies. This allows them to be tested on the PC using `pio test -e native`.
- **15-second inactivity timeout** — A short timeout was chosen so the ACTIVE/INACTIVE transition is easy to observe in the Wokwi simulation.

## Limitations

- **Simulation only** — The system was tested only in Wokwi. It was not validated on real ESP32 hardware.
- **No encoder interrupts** — The rotary encoder is read by polling in InputTask every 10 ms. On real hardware, a GPIO interrupt would be more power-efficient and would respond faster.
- **LDR is relative, not calibrated** — The light sensor is reported as a 0–100 % relative value. It is not calibrated in lux, so the numbers cannot be compared to a real light meter.
- **Only one measurement at a time** — The OLED shows one measurement per page. There is no combined view that shows all values at once.
- **`cppcheck` false positives** — `pio check` reports 20 `unusedFunction` warnings that are false positives caused by `cppcheck` analyzing each `.c` file separately.
- **No Wi-Fi or remote monitoring** — The system is fully local. There is no way to view the data remotely.
- **No persistent configuration** — Settings such as the temperature limits and the inactivity timeout are compiled into the firmware. They cannot be changed at runtime.

## Future Improvements

- **Use GPIO interrupts for the encoder** — Instead of polling, attach an interrupt to the encoder CLK pin. This frees up CPU time and responds faster.
- **Calibrate the LDR in lux** — Add a proper conversion from the ADC reading to lux using a known reference.
- **Add Wi-Fi monitoring** — Stream the sensor data to a simple web dashboard so the room can be monitored remotely.
- **Persistent configuration with NVS** — Allow the temperature limits and inactivity timeout to be changed at runtime and stored in non-volatile memory.
- **Watchdog for the tasks** — Add an ESP-IDF Task Watchdog to detect if a task ever becomes stuck.

## References and Acknowledgments

**References:**

- ESP-IDF Programming Guide — https://docs.espressif.com/projects/esp-idf/
- FreeRTOS Reference Manual — https://www.freertos.org/
- Wokwi ESP32 Simulator — https://wokwi.com/
- PlatformIO Documentation — https://docs.platformio.org/
- SSD1306 OLED datasheet
- DHT22 datasheet

**Acknowledgments:**

- Course: BCA152 — Real-Time Operating Systems
- Instructor / Collaborator: Paul Rodolf P. Castor
- Repository: https://github.com/aizakatebuale-crypto/BCA152-FreeRTOS-BasedESP32-Room-Multisensor