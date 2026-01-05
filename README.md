# 🍋📡 HelLo Juicer

**The Smart Chain Oiler & Anti-Theft Tracker.**

Based on the **Heltec T114 (nRF52840 + SX1262)**, this project combines intelligent chain maintenance with LoRaWAN-based theft protection.

## Features

*   **Chain Oiler:** Speed-dependent lubrication (GPS).
*   **Anti-Theft:** Always-on "Sentry Mode" with ultra-low power consumption.
*   **Connectivity:**
    *   **BLE:** Configuration via Smartphone App.
    *   **LoRaWAN:** Long-range theft alerts and status updates (The Things Network).
*   **Power:** Runs on 12V (Ignition) + LiPo Buffer Battery (Standby).

## Hardware

*   **MCU:** Heltec T114 (nRF52840)
*   **LoRa:** SX1262 (Onboard)
*   **GPS:** External Module (e.g. BN-220)
*   **IMU:** BNO085 (External)
*   **Driver:** MOSFET for Pump

## Architecture

This project shares the core logic (`Oiler`, `ImuHandler`) with the original [Chain Juicer](https://github.com/TechwriterSSchmidt/ChainJuicer) project but uses a different connectivity stack (BLE/LoRa instead of WiFi).
