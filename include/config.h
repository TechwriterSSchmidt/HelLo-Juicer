#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Firmware Version ---
#define FIRMWARE_VERSION "1.1.0"

// --- Hardware Pins (Heltec T114 / nRF52840) ---
#define LORA_NSS 29
#define LORA_DIO1 45
#define LORA_NRST 44
#define LORA_BUSY 43

#define GPS_RX_PIN 42
#define GPS_TX_PIN 40

#define IMU_SDA 26
#define IMU_SCL 27
#define IMU_INT_PIN 35 // Interrupt Pin for Wakeup

#define PUMP_PIN 6 
#define LED_PIN 34 // Builtin LED or external

#define IGNITION_PIN 5 // Input to detect 12V (via divider)
#define BATTERY_PIN 4  // ADC for Battery Voltage
#define USER_BUTTON_PIN 0 // Boot Button (P0.00)

// --- Power Management ---
#define COOLDOWN_TIME_MS (5 * 60 * 60 * 1000) // 5 Hours Listening Mode
#define EXTENSION_TIME_MS (1 * 60 * 60 * 1000) // +1 Hour on interaction
#define HEARTBEAT_INTERVAL_MS (15 * 60 * 1000) // 15 Minutes (in Cooldown)
#define SENTRY_HEARTBEAT_MS (6 * 60 * 60 * 1000) // 6 Hours (in Deep Sleep)

// --- Garage / Home Settings ---
#define HOME_RADIUS_M 50.0          // Garage Opener Trigger
#define HOME_PRE_ARRIVAL_RADIUS_M 500.0 // AI Stats Trigger (send before arrival)

// --- LoRaWAN Events ---
#define EVENT_IGNITION 1
#define EVENT_HOME 2
#define EVENT_SESSION_STATS 5

// --- Oiler Settings ---
#define MIN_SPEED_KMH 7.0
#define MAX_SPEED_KMH 250.0
#define MIN_ODOMETER_SPEED_KMH 2.0

// Pump Settings
#define PUMP_USE_PWM true
#define PUMP_PWM_FREQ 5000
#define PUMP_PWM_RESOLUTION 8
#define PUMP_RAMP_UP_MS 0       // Soft-Start duration (0 = Hard Kick to prevent whine)
#define PUMP_RAMP_DOWN_MS 0     // Soft-Stop duration
#define PUMP_SAFETY_CUTOFF_MS 30000

#define PULSE_DURATION_MS 55
#define PAUSE_DURATION_MS 2000

#define BLEEDING_DURATION_MS 20000 // Pumping time in ms for bleeding
#define BLEEDING_PULSE_MS 60       // Pulse duration for bleeding
#define BLEEDING_PAUSE_MS 320      // Pause duration for bleeding

// LED Settings
#define NUM_LEDS 1
#define LED_BRIGHTNESS_DIM 64
#define LED_BRIGHTNESS_HIGH 153

#endif
