# 📡 HelLo Juicer v0.2.0 (nRF52 + LoRaWAN)

**The Connected Guardian for your Motorcycle Chain.**

The **HelLo Juicer** ("Heltec LoRa Juicer") is the next evolution of the Chain Juicer project. Built on the powerful **Heltec T114** (nRF52840 + SX1262), it combines the proven GPS-controlled chain lubrication of its predecessor with long-range **LoRaWAN connectivity** and advanced **Anti-Theft** features.

**Why "HelLo Juicer"?**
*   **Hel**tec Hardware
*   **Lo**RaWAN Connectivity
*   **Juicer** Heritage

> **Status:** **Beta**
> The core oiling logic, LoRaWAN connectivity, and the Sentry Mode (Deep Sleep) are fully implemented. Field testing is currently in planning.

## Table of Contents
* [Features](#-features)
* [Power Management & Sentry Mode](#-power-management--sentry-mode)
* [LoRaWAN & Connectivity](#-lorawan--connectivity)
* [Hardware](#-hardware)
* [Installation](#-installation)
* [Configuration](#-configuration)
* [License](#license)

## Features

| Feature | Description | Details |
| :--- | :--- | :--- |
| **Speed-Dependent Oiling** | 5 configurable speed ranges. | Identical logic to Chain Juicer v2.0. Optimized for precision. |
| **LoRaWAN Telemetry** | Long-range status updates. | Sends Odometer, Tank Level, and Battery Voltage to **The Things Network (TTN)** -> Home Assistant. |
| **Anti-Theft Alarm** | Deep Sleep Sentry Mode. | Wakes up on motion, gets GPS fix, and sends Alarm. |
| **Smart Power** | 3-Stage Power Management. | Drive -> Cooldown (Listen) -> Sentry (Deep Sleep). |
| **Rain Mode** | Doubles oil amount in wet conditions. | **Button:** 1x Click. **Auto-Off:** 30 min or restart. |
| **Chain Flush Mode** | Intensive oiling for cleaning. | **Button:** 4x Click. |
| **Offroad Mode** | Time-based oiling. | **Button:** 3x Click. |
| **Tank Monitor** | Virtual oil level tracking. | Warns via LED and LoRaWAN when low. |
| **Garage Opener** | Smart Home Integration. | Sends LoRaWAN events on **Ignition** and **Home Arrival** (Geofence). |
| **AI Optimization** | Adaptive Learning. | Generates prompts for AI analysis and applies settings with confidence weighting. |

## Power Management & Sentry Mode

The HelLo Juicer features a sophisticated state machine to protect your bike's battery while remaining vigilant.

1.  **Drive Mode (Ignition ON):**
    *   Full power. GPS and Oiler are active.
    *   LoRaWAN sends status updates every 5 minutes.

2.  **Cooldown Mode (Ignition OFF -> 3 Hours):**
    *   System stays in "Light Sleep".
    *   Sends a "Heartbeat" every 15 minutes.
    *   **Remote Config:** This is the window to send LoRaWAN Downlinks to change settings.

3.  **Sentry Mode (After 3 Hours):**
    *   System enters **Deep Sleep (System OFF)**.
    *   Power consumption is negligible.
    *   **Wake-on-Motion:** The IMU (BNO085) remains active in low-power mode. If the bike is moved, the system wakes up immediately.

4.  **Alarm Mode:**
    *   Triggered by motion in Sentry Mode.
    *   Acquires GPS lock and transmits coordinates via LoRaWAN.

## LoRaWAN & Connectivity

The HelLo Juicer connects your motorcycle to the **Internet of Things**.

### Telemetry (Uplink)
The system periodically sends a compact byte-payload to TTN containing:
*   **Battery Voltage:** Monitor your bike's battery health remotely.
*   **Oil Tank Level:** Know when to refill before you ride.
*   **Odometer:** Track total distance driven.

### Integration
Data from TTN can be easily forwarded to **Home Assistant** via MQTT.

**Notifications:**
*   **Theft Alert:** Get a critical push notification on your phone immediately when the bike moves.
*   **Maintenance:** Receive a friendly reminder when the oil tank is low or the battery voltage drops.

### AI-Assisted Tuning (Conversational Maintenance)
Instead of manually tweaking numbers, use **Home Assistant Assist (LLM)** to talk to your bike:

1.  **Voice Feedback:** Tell your smart home: *"The chain looked a bit dry after today's ride."*
2.  **AI Analysis:** The LLM analyzes your feedback, considers the current settings, and decides on an adjustment (e.g., "Decrease interval by 10%").
3.  **Auto-Config:** Home Assistant sends the new optimized interval back to the HelLo Juicer via **LoRaWAN Downlink**.

## Anti-Theft System

When the bike is parked (Ignition OFF), the HelLo Juicer enters **Sentry Mode**.

1.  **Geofence:** The system remembers the GPS coordinates where it was parked.
2.  **Motion Sense:** The onboard IMU (or external sensor) monitors for movement/vibration.
3.  **Trigger:** If the bike moves > 50m OR detects sustained motion without ignition:
    *   **Immediate LoRaWAN Alarm:** Sends a high-priority uplink with GPS coordinates.
    *   **Local Alarm:** (Optional) Triggers a siren/horn output.

## Hardware

*   **MCU:** Heltec T114 (nRF52840 + SX1262 LoRa)
*   **GPS:** Standard NMEA GPS Module (UART)
*   **IMU:** BNO085 (Optional, for advanced motion detection)
*   **Power:** 12V to 5V Buck Converter (Automotive grade recommended)
*   **Driver:** MOSFET or Relay for Pump control.

## Installation

*(Coming Soon: Wiring diagrams and 3D printed housing files)*

## Configuration

Unlike the ESP32 version, the HelLo Juicer does not host a WiFi Web Interface. Instead, it uses **Bluetooth Low Energy (BLE)**.

1.  Download a BLE Terminal App (e.g., Adafruit Bluefruit Connect).
2.  Connect to "HelLo-Juicer".
3.  Use the command-line interface to set parameters:
    *   `set_dist 500` (Set interval to 500m)
    *   `set_rain on` (Toggle Rain Mode)
    *   `status` (View current stats)

---
*Based on the original Chain Juicer by TechwriterSSchmidt.*

## License

This project is licensed under the **PolyForm Noncommercial License 1.0.0**.

- **Noncommercial Use**: You may use this software for personal, educational, or evaluation purposes.
- **Commercial Use Restricted**: You may NOT use this software for commercial purposes (selling, paid services, business use) without prior written consent.

<details>
<summary>View Full License Text</summary>

### PolyForm Noncommercial License 1.0.0

#### 1. Purpose
This license allows you to use the software for noncommercial purposes.

#### 2. Agreement
In order to receive this license, you must agree to its rules. The rules of this license are both obligations (like a contract) and conditions to your license. You must not do anything with this software that triggers a rule that you cannot or will not follow.

#### 3. License Grant
The licensor grants you a copyright license for the software to do everything you might do with the software that would otherwise infringe the licensor's copyright in it for any permitted purpose. However, you may only do so to the extent that such use does not violate the rules.

#### 4. Permitted Purpose
A purpose is a permitted purpose if it consists of:
1. Personal use
2. Evaluation of the software
3. Development of software using the software as a dependency or evaluation tool
4. Educational use

**Commercial use is strictly prohibited without prior written consent from the author.**

#### 5. Rules

##### 5.1. Noncommercial Use
You must not use the software for any commercial purpose. A commercial purpose includes, but is not limited to:
1. Using the software to provide a service to third parties for a fee.
2. Selling the software or a derivative work.
3. Using the software in a commercial environment or business workflow.

##### 5.2. Notices
You must ensure that anyone who gets a copy of any part of the software from you also gets a copy of these terms or the URL for them above, as well as copies of any copyright notice or other rights notice in the software.

#### 6. Disclaimer
**THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.**

</details>
