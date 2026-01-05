# Release Notes

## v0.2.0 - The Sentry Update

This release marks a major milestone in the transition to the nRF52 platform (Heltec T114). The core focus was on implementing a robust power management state machine and the "Sentry" anti-theft system.

### New Features

#### Power Management State Machine
The system now operates in four distinct states to balance functionality and power consumption:
*   **Drive Mode:** Full power. Activated by ignition (12V). GPS, Oiler, and LoRaWAN are active.
*   **Cooldown Mode:** Active for 3 hours after parking. The system stays in "Light Sleep" to listen for LoRaWAN downlinks (configuration updates) and sends heartbeats every 15 minutes.
*   **Sentry Mode:** Activated after Cooldown expires. The system enters "Deep Sleep" (System OFF). Power consumption is negligible.
*   **Alarm Mode:** Triggered by significant motion during Sentry Mode. Wakes up, acquires GPS lock, and transmits coordinates via LoRaWAN.

#### LoRaWAN Integration
*   **Class A Device:** Optimized for battery powered operation.
*   **Heartbeats:** Periodic transmission of Battery Voltage, Tank Level, and Odometer.
*   **Alarm Uplinks:** High-priority transmission of GPS coordinates upon theft detection.
*   **Remote Configuration:** Downlink support to change settings (e.g., heartbeat interval) during the Cooldown phase.

#### Hardware Abstraction
*   **IMU Driver (BNO085):** Refactored to be platform-independent. Now supports "Wake-on-Motion" by triggering the nRF52 interrupt pin.
*   **Persistence:** Settings are now saved to the nRF52's internal flash file system (InternalFS), replacing the ESP32 Preferences library.

### Improvements
*   **Battery Monitoring:** Added voltage divider reading logic for the Heltec T114.
*   **GPS Handling:** Optimized GPS power usage during Alarm state (60s timeout).

### Known Issues / To-Do
*   **LoRa Keys:** The firmware currently uses placeholder keys (0000...). Users must update `main.cpp` or `secrets.h` with valid TTN credentials.
*   **BLE:** Bluetooth Low Energy configuration interface is currently disabled/pending implementation.
