#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include <TinyGPS++.h>
#include <Adafruit_BNO08x.h>
#include "NrfPersistence.h"
#include "LoraWanHandler.h"
#include "Oiler.h"
#include "ImuHandler.h"

// --- Pin Definitions (Heltec T114 / nRF52840) ---
// Note: Verify these against the T114 Schematic!
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

// --- Constants ---
#define COOLDOWN_TIME_MS (3 * 60 * 60 * 1000) // 3 Hours
#define HEARTBEAT_INTERVAL_MS (15 * 60 * 1000) // 15 Minutes (in Cooldown)
#define SENTRY_HEARTBEAT_MS (6 * 60 * 60 * 1000) // 6 Hours (in Deep Sleep)

// --- Objects ---
SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);
LoraWanHandler lora(&radio);
TinyGPSPlus gps;
NrfPersistence persistence;
Oiler oiler(&persistence, PUMP_PIN, LED_PIN, -1); // No Temp Sensor for now
// ImuHandler imuHandler; // TODO: Integrate ImuHandler properly

// --- State Machine ---
enum SystemState {
    STATE_BOOT,
    STATE_DRIVE,    // Ignition ON: Full Power
    STATE_COOLDOWN, // Ignition OFF: Listening Mode (2-3h)
    STATE_SENTRY,   // Deep Sleep: Wake on Motion
    STATE_ALARM     // Motion Detected
};

SystemState currentState = STATE_BOOT;
unsigned long stateStartTime = 0;
unsigned long lastHeartbeat = 0;

// --- Helpers ---
float readBatteryVoltage() {
    // T114 specific: Read ADC, apply divider math
    // Example: 12-bit ADC, 3.3V ref, Divider 1/2
    int raw = analogRead(BATTERY_PIN);
    return (raw / 4095.0) * 3.3 * 2.0; // Adjust multiplier based on hardware
}

bool isIgnitionOn() {
    return digitalRead(IGNITION_PIN) == HIGH;
}

void setup() {
    Serial.begin(115200);
    delay(2000); // Safety delay
    Serial.println("HelLo Juicer - Booting...");

    // 1. Init Pins
    pinMode(IGNITION_PIN, INPUT); // Add Pull-down externally if needed
    pinMode(IMU_INT_PIN, INPUT_PULLUP);
    pinMode(BATTERY_PIN, INPUT);

    // 2. Init Components
    persistence.begin("hello", false);
    
    // LoRa
    // TODO: Load Keys from Persistence or Secrets
    lora.setAppEui("0000000000000000"); 
    lora.setDevEui("0000000000000000");
    lora.setAppKey("00000000000000000000000000000000");
    if (!lora.begin()) {
        Serial.println("LoRa Init Failed!");
    }
    lora.join(); // Blocking join for now

    // Oiler
    oiler.begin(IMU_SDA, IMU_SCL);

    // GPS
    Serial1.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    // 3. Initial State
    if (isIgnitionOn()) {
        currentState = STATE_DRIVE;
    } else {
        currentState = STATE_COOLDOWN; // Start in Cooldown if booted on battery
    }
    stateStartTime = millis();
}

void loop() {
    unsigned long now = millis();

    // Global: Handle LoRa Downlinks/Events
    lora.loop();

    switch (currentState) {
        // ---------------------------------------------------------
        // DRIVE MODE: Full Functionality
        // ---------------------------------------------------------
        case STATE_DRIVE:
            // 1. Check Ignition
            if (!isIgnitionOn()) {
                Serial.println("Ignition OFF -> Entering Cooldown Mode");
                currentState = STATE_COOLDOWN;
                stateStartTime = now;
                lastHeartbeat = 0; // Force immediate heartbeat
                break;
            }

            // 2. GPS Update
            while (Serial1.available()) {
                gps.encode(Serial1.read());
            }
            
            // 3. Oiler Logic
            if (gps.location.isUpdated() || gps.speed.isUpdated()) {
                oiler.update(gps.speed.kmph(), gps.location.lat(), gps.location.lng(), true);
            }
            oiler.loop();

            // 4. Periodic Status Update (e.g. every 5 mins)
            if (now - lastHeartbeat > (5 * 60 * 1000)) {
                lora.sendStatus(readBatteryVoltage(), oiler.currentTankLevelMl, oiler.getTotalDistance());
                lastHeartbeat = now;
            }
            break;

        // ---------------------------------------------------------
        // COOLDOWN MODE: Listening for 3h
        // ---------------------------------------------------------
        case STATE_COOLDOWN:
            // 1. Check Ignition
            if (isIgnitionOn()) {
                Serial.println("Ignition ON -> Drive Mode");
                currentState = STATE_DRIVE;
                break;
            }

            // 2. Check Timeout (3h)
            if (now - stateStartTime > COOLDOWN_TIME_MS) {
                Serial.println("Cooldown Expired -> Entering Sentry Mode (Deep Sleep)");
                currentState = STATE_SENTRY;
                break;
            }

            // 3. Heartbeat & Listen
            // Send status frequently to open RX windows (Class A)
            if (now - lastHeartbeat > HEARTBEAT_INTERVAL_MS) {
                Serial.println("Cooldown Heartbeat...");
                lora.sendStatus(readBatteryVoltage(), oiler.currentTankLevelMl, oiler.getTotalDistance());
                lastHeartbeat = now;
            }
            
            // Light Sleep could be added here to save power between loops
            delay(100); 
            break;

        // ---------------------------------------------------------
        // SENTRY MODE: Deep Sleep & Alarm
        // ---------------------------------------------------------
        case STATE_SENTRY:
            // 1. Prepare for Sleep
            // Configure IMU for Motion Interrupt
            oiler.imu.enableMotionInterrupt();
            
            Serial.println("Going to System OFF...");
            delay(100);
            
            // 2. Setup Wakeup Sources
            // Wake on IMU INT pin (Active Low usually)
            // Ensure the pin is configured as input with pullup
            pinMode(IMU_INT_PIN, INPUT_PULLUP);
            // Configure SENSE mechanism (Low level triggers wakeup)
            nrf_gpio_cfg_sense_input(digitalPinToPinName(IMU_INT_PIN), NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
            
            // 3. Enter Deep Sleep
            NRF_POWER->SYSTEMOFF = 1;
            
            // ... Sleeping ...
            // Code execution stops here. Wakeup is a Reset.
            while(1);
            // Reset usually happens here.
            // If we are here, it might be a "fake" sleep for testing
            if (isIgnitionOn()) {
                currentState = STATE_DRIVE;
            } else {
                // Assume Motion woke us up
                currentState = STATE_ALARM;
            }
            break;

        // ---------------------------------------------------------
        // ALARM MODE: Tracking
        // ---------------------------------------------------------
        case STATE_ALARM:
            Serial.println("ALARM! Motion Detected!");
            
            // 1. Try to get GPS Fix
            // Power up GPS
            unsigned long alarmStart = millis();
            bool fixFound = false;
            
            while (millis() - alarmStart < 60000) { // Try for 60s
                while (Serial1.available()) gps.encode(Serial1.read());
                if (gps.location.isValid()) {
                    fixFound = true;
                    break;
                }
            }
            
            // 2. Send Alarm Packet
            lora.sendAlarm(gps.location.lat(), gps.location.lng());
            
            // 3. Return to Sentry (or Cooldown?)
            // Maybe stay awake for a bit to track?
            currentState = STATE_COOLDOWN; // Go back to listening/tracking for a while
            stateStartTime = millis(); // Reset Cooldown timer
            break;
    }
}
