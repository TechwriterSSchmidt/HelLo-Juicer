#ifndef NRF_PERSISTENCE_H
#define NRF_PERSISTENCE_H

#include "Persistence.h"
#include <Arduino.h>

class NrfPersistence : public IPersistence {
private:
    String _namespace;
    String getFilePath(const char* key);

public:
    void begin(const char* namespaceName, bool readOnly) override;
    void end() override;
    void clear() override;
    
    void putInt(const char* key, int32_t value) override;
    int32_t getInt(const char* key, int32_t defaultValue) override;
    
    void putUInt(const char* key, uint32_t value) override;
    uint32_t getUInt(const char* key, uint32_t defaultValue) override;

    void putFloat(const char* key, float value) override;
    float getFloat(const char* key, float defaultValue) override;
    
    void putDouble(const char* key, double value) override;
    double getDouble(const char* key, double defaultValue) override;

    void putBool(const char* key, bool value) override;
    bool getBool(const char* key, bool defaultValue) override;
    
    void putUChar(const char* key, uint8_t value) override;
    uint8_t getUChar(const char* key, uint8_t defaultValue) override;

    void putBytes(const char* key, const void* value, size_t len) override;
    size_t getBytes(const char* key, void* buf, size_t maxLen) override;
    size_t getBytesLength(const char* key) override;
};

#endif
