#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include <TinyGPS++.h>
#include <Adafruit_BNO08x.h>
#include "config.h" // Include the new config file
#include "NrfPersistence.h"
#include "LoraWanHandler.h"
#include "Oiler.h"
#include "ImuHandler.h"

// --- Objects ---
SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);
LoraWanHandler lora(&radio);
TinyGPSPlus gps;
NrfPersistence persistence;
Oiler oiler(&persistence, PUMP_PIN, LED_PIN, -1); // No Temp Sensor for now
// ImuHandler imuHandler; // TODO: Integrate ImuHandler properly

// --- Callbacks ---
void onHomeConfig(double lat, double lon) {
    Serial.printf("Main: Updating Home Coordinates to %.6f, %.6f\n", lat, lon);
    homeLat = lat;
    homeLon = lon;
    
    persistence.begin("hello", false);
    persistence.putDouble("home_lat", homeLat);
    persistence.putDouble("home_lon", homeLon);
    persistence.end();
}

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
unsigned long cooldownEndTime = 0;
unsigned long lastHeartbeat = 0;
bool homeArrivalSent = false;
bool sessionStatsSent = false;

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

    // Check Reset Reason (Wake from System OFF?)
    uint32_t resetReason = NRF_POWER->RESETREAS;
    NRF_POWER->RESETREAS = 0xFFFFFFFF; // Clear flags
    Serial.printf("Reset Reason: 0x%08X\n", resetReason);
    
    // Bit 16 (0x10000) = Wake up from System OFF (GPIO Detect)
    bool wokeFromSleep = (resetReason & 0x00010000);

    // 1. Init Pins
    pinMode(IGNITION_PIN, INPUT); // Add Pull-down externally if needed
    pinMode(IMU_INT_PIN, INPUT_PULLUP);
    pinMode(USER_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BATTERY_PIN, INPUT);

    // 2. Init Components
    persistence.begin("hello", false);
    homeLat = persistence.getDouble("home_lat", 0.0);
    homeLon = persistence.getDouble("home_lon", 0.0);
    Serial.printf("Home Coords: %.6f, %.6f\n", homeLat, homeLon);
    
    // LoRa
    // TODO: Load Keys from Persistence or Secrets
    lora.setAppEui("0000000000000000"); 
    lora.setDevEui("0000000000000000");
    lora.setAppKey("00000000000000000000000000000000");
    lora.setHomeConfigCallback(onHomeConfig);
    
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
        // Send Ignition Event immediately on startup
        lora.sendEvent(EVENT_IGNITION);
    } el// Check Wakeup Source
        if (digitalRead(USER_BUTTON_PIN) == LOW) {
            Serial.println("Wakeup: Button -> Listening Mode");
            currentState = STATE_COOLDOWN;
            cooldownEndTime = millis() + COOLDOWN_TIME_MS;
        } else if (digitalRead(IMU_INT_PIN) == LOW) {
            Serial.println("Wakeup: Motion -> ALARM!");
            currentState = STATE_ALARM;
        } else {
            // Fallback
            Serial.println("Wakeup: Unknown -> Sentry");
            currentState = STATE_SENTRY;
        }
    } else {
        currentState = STATE_SENTRY; // Default to Sleep if not Ignition
        currentState = STATE_COOLDOWN; // Start in Cooldown if booted on battery (Manual Reset)
    }
    stateStartTime = millis();
}

void loop() {
    unsigned long now = millis();Sentry Mode immediately");
                currentState = STATE_SENTRY;
                stateStartTime = now;
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
                homeArrivalSent = false; // Reset for next ride
                break;
            }

            // 2. GPS Update
            while (Serial1.available()) {
                gps.encode(Serial1.read());
            }
            
            // 3. Oiler Logic
            if (gps.location.isUpdated() || gps.speed.isUpdated()) {
                oiler.update(gps.speed.kmph(), gps.location.lat(), gps.location.lng(), true);
                
                // Garage Opener & AI Stats Logic
                if (gps.location.isValid() && homeLat != 0.0 && homeLon != 0.0) {
                    double distToHome = TinyGPSPlus::distanceBetween(
                        gps.location.lat(), gps.location.lng(),
                        homeLat, homeLon
                    );
                    
                    // 1. Pre-Arrival: Send AI Stats (e.g. 500m before)
                    if (distToHome < HOME_PRE_ARRIVAL_RADIUS_M && !sessionStatsSent) {
                        Serial.println("Approaching Home! Sending Session Stats for AI...");
                        lora.sendSessionStats(oiler.getSessionStats(), NUM_RANGES);
                        sessionStatsSent = true;
                    }

                    // 2. Arrival: Open Garage (e.g. 50m)
                    if (distToHome < HOME_RADIUS_M && !homeArrivalSent) {
                        Serial.println("Arrived Home! Sending Garage Signal...");
                        lora.sendEvent(EVENT_HOME);
                        homeArrivalSent = true;
                    } 
                    
                    // Reset flags if we move away (Hysteresis)
                    if (distToHome > (HOME_PRE_ARRIVAL_RADIUS_M * 1.5)) {
                        sessionStatsSent = false;
                        homeArrivalSent = false;
                    }
                }
            }
            oiler.loop();

            // 4. Periodic Status Update (e.g. every 5 mins)
            if (now - lastHeartbeat > (5 * 60 * 1000)) {
                lora.sendStatus(readBatteryVoltage(), oiler.currentTankLevelMl, oiler.getTotalDistance());
                lastHeartbeat = now;
            }
            break;

        // ---------------------------------------------------------
        // COOLDOWN MODE: Listening (Manual Activation)
        // ---------------------------------------------------------
        case STATE_COOLDOWN:
            // 1. Check Ignition
            if (isIgnitionOn()) {
                Serial.println("Ignition ON -> Drive Mode");
                currentState = STATE_DRIVE;
                lora.sendEvent(EVENT_IGNITION); // Send Ignition Event
                break;
            }

            // 2. Check Interaction (Extend Timeout)
            if (lora.downlinkReceived) {
                lora.downlinkReceived = false;
                unsigned long oneHourFromNow = now + EXTENSION_TIME_MS;
                if (oneHourFromNow > cooldownEndTime) {
                    cooldownEndTime = oneHourFromNow;
                    Serial.println("Interaction detected! Timeout extended.");
                }
            }

            // 3. Check Timeout
            if (now > cooldownEndTime) {
                Serial.println("Listening Timeout -> Entering Sentry Mode");
                currentState = STATE_SENTRY;
                break;
            }

            // 4break;
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
            
            pinMode(USER_BUTTON_PIN, INPUT_PULLUP);

            // Configure SENSE mechanism (Low level triggers wakeup)
            nrf_gpio_cfg_sense_input(digitalPinToPinName(IMU_INT_PIN), NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
            nrf_gpio_cfg_sense_input(digitalPinToPinName(USER_BUTTON
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
