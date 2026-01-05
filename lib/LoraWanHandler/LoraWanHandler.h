#ifndef LORAWAN_HANDLER_H
#define LORAWAN_HANDLER_H

#include <Arduino.h>
#include <RadioLib.h>

// Region specific settings (EU868)
#define LORA_REGION_EU868

class LoraWanHandler {
public:
    LoraWanHandler(SX1262* radioModule);
    
    // Lifecycle
    bool begin();
    void loop();

    // LoRaWAN Actions
    bool join();
    bool isJoined() { return _joined; }
    
    // Sending Data
    void sendStatus(float voltage, float tankLevel, float totalDistance);
    void sendAlarm(double lat, double lon);
    
    // Downlink / Remote Config
    void setConfigCallback(void (*callback)(uint32_t newInterval));
    void checkDownlink(); // Call periodically or after TX

    // Configuration
    void setAppEui(const char* appEui);
    void setAppKey(const char* appKey);
    void setDevEui(const char* devEui);

private:
    SX1262* _radio;
    bool _joined = false;
    
    // Keys (Buffers)
    uint8_t _appEui[8];
    uint8_t _devEui[8];
    uint8_t _appKey[16];
    uint8_t _nwkKey[16]; // For LoRaWAN 1.1, usually same as AppKey for 1.0

    // Callback for config updates
    void (*_configCallback)(uint32_t) = nullptr;

    // Helper to convert hex string to byte array
    void strToBytes(const char* str, uint8_t* bytes, size_t len);
    
    // Payload Encoder (CayenneLPP style or Custom)
    void encodeStatus(uint8_t* buffer, size_t& len, float voltage, float tankLevel, float totalDistance);
    void encodeAlarm(uint8_t* buffer, size_t& len, double lat, double lon);
};

#endif
