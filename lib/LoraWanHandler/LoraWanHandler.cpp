#include "LoraWanHandler.h"

LoraWanHandler::LoraWanHandler(SX1262* radioModule) {
    _radio = radioModule;
    _node = new LoRaWANNode(radioModule, &EU868);
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
    // RadioLib's LoRaWANNode handles most things in sendReceive, 
    // but if we used interrupts we might need to check flags here.
    // For now, we are blocking/polling in send methods.
}

bool LoraWanHandler::join() {
    Serial.println("LoRa: Attempting to join TTN...");
    
    // Configure LoRaWAN Node
    _node->beginOTAA(_joinEui, _devEui, _nwkKey, _appKey);
    
    int state = _node->activateOTAA();
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("LoRa: Joined!");
        _joined = true;
    } else {
        Serial.printf("LoRa: Join failed, code %d\n", state);
        _joined = false;
    }
    
    return _joined;
}

void LoraWanHandler::sendStatus(float voltage, float tankLevel, float totalDistance) {
    if (!_joined) return;
    
    uint8_t buffer[32];
    size_t len = 0;
    encodeStatus(buffer, len, voltage, tankLevel, totalDistance);
    
    Serial.println("LoRa: Sending Status Update...");
    
    // Send uplink
    int state = _node->sendReceive(buffer, len);
    
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("LoRa: TX Success");
        
        // Check for downlink
        if (_node->downlinkLength > 0) {
            Serial.println("LoRa: Received Downlink!");
            processDownlink(_node->downlinkData, _node->downlinkLength);
        }
    } else {
        Serial.printf("LoRa: TX Failed, code %d\n", state);
        // If we get a "network not joined" error, reset flag
        if (state == RADIOLIB_ERR_NETWORK_NOT_JOINED) {
            _joined = false;
        }
    }
}

void LoraWanHandler::sendAlarm(double lat, double lon) {
    if (!_joined) return;

    uint8_t buffer[32];
    size_t len = 0;
    encodeAlarm(buffer, len, lat, lon);
    
    Serial.println("LoRa: Sending ALARM!");
    
    // Send uplink (Confirmed for Alarm?)
    // For now unconfirmed to save airtime/duty cycle
    int state = _node->sendReceive(buffer, len);
    
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("LoRa: Alarm Sent");
    } else {
        Serial.printf("LoRa: Alarm TX Failed, code %d\n", state);
    }
}

void LoraWanHandler::sendEvent(uint8_t eventId) {
    if (!_joined) return;

    uint8_t buffer[32];
    size_t len = 0;
    encodeEvent(buffer, len, eventId);
    
    Serial.printf("LoRa: Sending Event %d...\n", eventId);
    
    int state = _node->sendReceive(buffer, len);
    
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("LoRa: Event Sent");
    } else {
        Serial.printf("LoRa: Event TX Failed, code %d\n", state);
    }
}

void LoraWanHandler::encodeStatus(uint8_t* buffer, size_t& len, float voltage, float tankLevel, float totalDistance) {
    // Simple Custom Protocol (CayenneLPP style or custom)
    // Using Custom for compactness
    // Byte 0: Type (0x01 = Status)
    // Byte 1: Voltage (x10) -> 12.5V = 125
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

void LoraWanHandler::encodeEvent(uint8_t* buffer, size_t& len, uint8_t eventId) {
    // Byte 0: Type (0x03 = EVENT)
    // Byte 1: Event ID (1=Ignition, 2=Home)
    
    buffer[0] = 0x03;
    buffer[1] = eventId;
    len = 2;
}

void LoraWanHandler::processDownlink(const uint8_t* data, size_t len) {
    if (len < 1) return;

    // Mark interaction
    downlinkReceived = true;

    // Protocol: Byte 0 = 0x02 (Config), Byte 1-2 = Interval (uint16_t)
    if (data[0] == 0x02 && len >= 3) {
        uint16_t newInterval = (data[1] << 8) | data[2];
        Serial.printf("LoRa: Received Downlink Config. New Interval: %d m\n", newInterval);
        if (_configCallback) {
            _configCallback(newInterval);
        }
    }
    // Protocol: Byte 0 = 0x04 (Set Home), Byte 1-4 = Lat, Byte 5-8 = Lon (int32 * 1M)
    else if (data[0] == 0x04 && len >= 9) {
        int32_t latI = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
        int32_t lonI = (data[5] << 24) | (data[6] << 16) | (data[7] << 8) | data[8];
        
        double lat = (double)latI / 1000000.0;
        double lon = (double)lonI / 1000000.0;
        
        Serial.printf("LoRa: Received Home Config. Lat: %.6f, Lon: %.6f\n", lat, lon);
        if (_homeConfigCallback) {
            _homeConfigCallback(lat, lon);
        }
    }
}

void LoraWanHandler::setConfigCallback(void (*callback)(uint32_t)) {
    _configCallback = callback;
}

void LoraWanHandler::setHomeConfigCallback(void (*callback)(double, double)) {
    _homeConfigCallback = callback;
}

void LoraWanHandler::setAppEui(const char* appEui) {
    _joinEui = strToUInt64(appEui);
}

void LoraWanHandler::setDevEui(const char* devEui) {
    _devEui = strToUInt64(devEui);
}

void LoraWanHandler::setAppKey(const char* appKey) {
    hexStringToBytes(appKey, _appKey, 16);
    // For LoRaWAN 1.0.x, NwkKey is usually the same as AppKey
    memcpy(_nwkKey, _appKey, 16);
}

// Helpers
uint64_t LoraWanHandler::strToUInt64(const char* str) {
    uint64_t value = 0;
    for (int i = 0; i < 16; i++) {
        char c = str[i];
        uint8_t nibble = 0;
        if (c >= '0' && c <= '9') nibble = c - '0';
        else if (c >= 'A' && c <= 'F') nibble = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') nibble = c - 'a' + 10;
        value = (value << 4) | nibble;
    }
    return value;
}

void LoraWanHandler::hexStringToBytes(const char* str, uint8_t* bytes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        char high = str[i*2];
        char low = str[i*2+1];
        
        uint8_t h = 0, l = 0;
        if (high >= '0' && high <= '9') h = high - '0';
        else if (high >= 'A' && high <= 'F') h = high - 'A' + 10;
        else if (high >= 'a' && high <= 'f') h = high - 'a' + 10;
        
        if (low >= '0' && low <= '9') l = low - '0';
        else if (low >= 'A' && low <= 'F') l = low - 'A' + 10;
        else if (low >= 'a' && low <= 'f') l = low - 'a' + 10;
        
        bytes[i] = (h << 4) | l;
    }
}
