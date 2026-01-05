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
    void sendEvent(uint8_t eventId); // 1=Ignition, 2=Home
    void sendSessionStats(uint32_t* timeInRanges, uint8_t numRanges); // Send AI Stats
    
    // Downlink / Remote Config
    void setConfigCallback(void (*callback)(uint32_t newInterval));
    void setHomeConfigCallback(void (*callback)(double lat, double lon));
    void checkDownlink(); // Call periodically or after TX

    bool downlinkReceived = false; // Flag to indicate interaction

    // Configuration
    void setAppEui(const char* appEui);
    void setAppKey(const char* appKey);
    void setDevEui(const char* devEui);

private:
    SX1262* _radio;
    LoRaWANNode* _node;
    bool _joined = false;
    
    // Keys
    uint64_t _joinEui; // AppEUI
    uint64_t _devEui;
    uint8_t _appKey[16];
    uint8_t _nwkKey[16]; 

    // Callback for config updates
    void (*_configCallback)(uint32_t) = nullptr;
    void (*_homeConfigCallback)(double, double) = nullptr;

    // Helpers
    uint64_t strToUInt64(const char* str);
    void hexStringToBytes(const char* str, uint8_t* bytes, size_t len);
    void processDownlink(const uint8_t* data, size_t len);
    
    // Payload Encoder (CayenneLPP style or Custom)
    void encodeStatus(uint8_t* buffer, size_t& len, float voltage, float tankLevel, float totalDistance);
    void encodeAlarm(uint8_t* buffer, size_t& len, double lat, double lon);
    void encodeEvent(uint8_t* buffer, size_t& len, uint8_t eventId);
};

#endif
