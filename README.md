# 📡 HelLo Juicer v0.2.0 (nRF52 + LoRaWAN)

**The Connected Guardian for your Motorcycle Chain.**

The **HelLo Juicer** ("Heltec LoRa Juicer") is the next evolution of the Chain Juicer project. Built on the powerful **Heltec T114** (nRF52840 + SX1262), it combines the proven GPS-controlled chain lubrication of its predecessor with long-range **LoRaWAN connectivity** and advanced **Anti-Theft** features.

**Why "HelLo Juicer"?**
*   **Hel**tec Hardware
*   **Lo**RaWAN Connectivity
*   **Juicer** Heritage

> **Status:** 🧪 **Beta**
> The core oiling logic, LoRaWAN connectivity, and the Sentry Mode (Deep Sleep) are fully implemented. Field testing is currently in progress.

## Table of Contents
* [Features](#-features)
* [Power Management & Sentry Mode](#-power-management--sentry-mode)
* [LoRaWAN & Connectivity](#-lorawan--connectivity)
* [Hardware](#-hardware)
* [Installation](#-installation)
* [Configuration](#-configuration)

## 🚀 Features

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

## 🔋 Power Management & Sentry Mode

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

## 🌐 LoRaWAN & Connectivity

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

### 🧠 AI-Assisted Tuning (Conversational Maintenance)
Instead of manually tweaking numbers, use **Home Assistant Assist (LLM)** to talk to your bike:

1.  **Voice Feedback:** Tell your smart home: *"The chain looked a bit dry after today's ride."*
2.  **AI Analysis:** The LLM analyzes your feedback, considers the current settings, and decides on an adjustment (e.g., "Decrease interval by 10%").
3.  **Auto-Config:** Home Assistant sends the new optimized interval back to the HelLo Juicer via **LoRaWAN Downlink**.

## 🛡️ Anti-Theft System

When the bike is parked (Ignition OFF), the HelLo Juicer enters **Sentry Mode**.

1.  **Geofence:** The system remembers the GPS coordinates where it was parked.
2.  **Motion Sense:** The onboard IMU (or external sensor) monitors for movement/vibration.
3.  **Trigger:** If the bike moves > 50m OR detects sustained motion without ignition:
    *   **Immediate LoRaWAN Alarm:** Sends a high-priority uplink with GPS coordinates.
    *   **Local Alarm:** (Optional) Triggers a siren/horn output.

## 🔧 Hardware

*   **MCU:** Heltec T114 (nRF52840 + SX1262 LoRa)
*   **GPS:** Standard NMEA GPS Module (UART)
*   **IMU:** BNO085 (Optional, for advanced motion detection)
*   **Power:** 12V to 5V Buck Converter (Automotive grade recommended)
*   **Driver:** MOSFET or Relay for Pump control.

## 📦 Installation

*(Coming Soon: Wiring diagrams and 3D printed housing files)*

## ⚙️ Configuration

Unlike the ESP32 version, the HelLo Juicer does not host a WiFi Web Interface. Instead, it uses **Bluetooth Low Energy (BLE)**.

1.  Download a BLE Terminal App (e.g., Adafruit Bluefruit Connect).
2.  Connect to "HelLo-Juicer".
3.  Use the command-line interface to set parameters:
    *   `set_dist 500` (Set interval to 500m)
    *   `set_rain on` (Toggle Rain Mode)
    *   `status` (View current stats)

---
*Based on the original Chain Juicer by TechwriterSSchmidt.*
