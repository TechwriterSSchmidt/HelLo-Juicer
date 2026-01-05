#include "LoraWanHandler.h"

LoraWanHandler::LoraWanHandler(SX1262* radioModule) {
    _radio = radioModule;
}

bool LoraWanHandler::begin() {
    // Initialize the radio
    // Note: Pin setup is handled in main.cpp or by the board definition
    int state = _radio->begin();
    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("LoRa: Init failed, code %d\n", state);
        return false;
    }
    
    Serial.println("LoRa: Radio initialized.");
    return true;
}

void LoraWanHandler::loop() {
    // Handle LoRaWAN state machine (re-join if lost, handle RX windows)
    // This is complex with RadioLib, usually requires an event-driven approach or polling
}

bool LoraWanHandler::join() {
    Serial.println("LoRa: Attempting to join TTN...");
    
    // Configure LoRaWAN Node
    // This is a placeholder. RadioLib requires setting up the LoRaWANNode instance.
    // For now, we just simulate the structure.
    
    // int state = node.activateOTAA();
    // ...
    
    _joined = true; // Sim
    return _joined;
}

void LoraWanHandler::sendStatus(float voltage, float tankLevel, float totalDistance) {
    if (!_joined) return;
    
    uint8_t buffer[32];
    size_t len = 0;
    encodeStatus(buffer, len, voltage, tankLevel, totalDistance);
    
    Serial.println("LoRa: Sending Status Update...");
    // node.sendReceive(buffer, len);
    
    // After sending, we should check for downlink (Class A device opens RX windows after TX)
    checkDownlink();
}

void LoraWanHandler::sendAlarm(double lat, double lon) {
    if (!_joined) return;

    uint8_t buffer[32];
    size_t len = 0;
    encodeAlarm(buffer, len, lat, lon);
    
    Serial.println("LoRa: Sending ALARM!");
    // node.sendReceive(buffer, len);
}

void LoraWanHandler::encodeStatus(uint8_t* buffer, size_t& len, float voltage, float tankLevel, float totalDistance) {
    // Simple Custom Protocol
    // Byte 0: Type (0x01 = Status)
    // Byte 1: Voltage (x10)
    // Byte 2: Tank Level (%)
    // Byte 3-6: Odometer (uint32_t, meters)
    
    buffer[0] = 0x01;
    buffer[1] = (uint8_t)(voltage * 10);
    buffer[2] = (uint8_t)tankLevel;
    
    uint32_t distMeters = (uint32_t)(totalDistance * 1000);
    buffer[3] = (distMeters >> 24) & 0xFF;
    buffer[4] = (distMeters >> 16) & 0xFF;
    buffer[5] = (distMeters >> 8) & 0xFF;
    buffer[6] = distMeters & 0xFF;
    
    len = 7;
}

void LoraWanHandler::encodeAlarm(uint8_t* buffer, size_t& len, double lat, double lon) {
    // Simple Custom Protocol
    // Byte 0: Type (0x99 = ALARM)
    // Byte 1-4: Lat (int32_t, x1000000)
    // Byte 5-8: Lon (int32_t, x1000000)
    
    buffer[0] = 0x99;
    
    int32_t latI = (int32_t)(lat * 1000000);
    int32_t lonI = (int32_t)(lon * 1000000);
    
    buffer[1] = (latI >> 24) & 0xFF;
    buffer[2] = (latI >> 16) & 0xFF;
    buffer[3] = (latI >> 8) & 0xFF;
    buffer[4] = latI & 0xFF;
    
    buffer[5] = (lonI >> 24) & 0xFF;
    buffer[6] = (lonI >> 16) & 0xFF;
    buffer[7] = (lonI >> 8) & 0xFF;
    buffer[8] = lonI & 0xFF;
    
    len = 9;
}

void LoraWanHandler::setConfigCallback(void (*callback)(uint32_t)) {
    _configCallback = callback;
}

void LoraWanHandler::checkDownlink() {
    // In a real RadioLib implementation, the downlink data is often returned 
    // by the sendReceive() call or available in a buffer after the RX window.
    
    // Simulation of receiving a "Set Interval" command
    // Protocol: Byte 0 = 0x02 (Config), Byte 1-2 = Interval (uint16_t)
    
    // uint8_t rxBuffer[256];
    // int rxLen = node.getDownlink(rxBuffer);
    
    // if (rxLen > 0 && rxBuffer[0] == 0x02) {
    //     uint16_t newInterval = (rxBuffer[1] << 8) | rxBuffer[2];
    //     Serial.printf("LoRa: Received Downlink Config. New Interval: %d m\n", newInterval);
    //     if (_configCallback) {
    //         _configCallback(newInterval);
    //     }
    // }
}

void LoraWanHandler::setAppEui(const char* appEui) {
    // strToBytes(appEui, _appEui, 8);
}
// ... Implement other setters
