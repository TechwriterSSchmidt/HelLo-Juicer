#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include <TinyGPS++.h>
#include <Adafruit_BNO08x.h>

// --- Configuration ---
// TODO: Define real pins for T114
#define LORA_NSS 8
#define LORA_DIO1 14
#define LORA_NRST 12
#define LORA_BUSY 13

#define GPS_RX_PIN 4
#define GPS_TX_PIN 5

#define IMU_SDA 26
#define IMU_SCL 27

#define PUMP_PIN 6 // Example Pin

// --- Objects ---
SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);
TinyGPSPlus gps;
Adafruit_BNO08x bno08x;

// --- State Machine ---
enum SystemState {
    STATE_BOOT,
    STATE_DRIVE,    // Ignition ON: Oiling active, GPS active, BLE active
    STATE_SENTRY,   // Ignition OFF: Deep Sleep, IMU active, LoRa waiting
    STATE_ALARM     // Motion detected in Sentry: LoRa TX, GPS Search
};

SystemState currentState = STATE_BOOT;
bool ignitionState = false;

void setup() {
    Serial.begin(115200);
    // while(!Serial); // Don't wait for serial in production!

    Serial.println("HelLo Juicer - Booting...");

    // 1. Init Hardware
    // Init LoRa
    // Init GPS
    // Init IMU
    // Init Pump Pin

    // 2. Check Ignition (Voltage Divider or USB Power)
    // ignitionState = checkIgnition();

    // 3. Restore Settings (InternalFS)
    
    currentState = STATE_DRIVE; // Default for testing
}

void loop() {
    switch (currentState) {
        case STATE_DRIVE:
            // Standard Oiler Logic
            // Update GPS
            // Update Oiler
            // Handle BLE
            
            // Check for Ignition OFF -> Transition to SENTRY
            break;

        case STATE_SENTRY:
            // Deep Sleep Logic
            // 1. Configure IMU for Wake-on-Motion
            // 2. Power down GPS & LoRa
            // 3. Go to System OFF / Deep Sleep
            // 4. On Wakeup (Reset or Interrupt) -> Check source
            break;

        case STATE_ALARM:
            // We woke up from Sentry due to motion!
            // 1. Send LoRa Alarm Packet
            // 2. Try to get GPS Fix
            // 3. Send Position
            // 4. Return to SENTRY
            break;
    }
}
