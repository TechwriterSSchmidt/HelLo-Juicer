#include "NrfPersistence.h"
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

// Helper to write simple types
template <typename T>
void writeValue(String path, T value) {
    // FILE_WRITE truncates existing file or creates new one
    File file = InternalFS.open(path.c_str(), FILE_WRITE);
    if (file) {
        file.write((uint8_t*)&value, sizeof(T));
        file.close();
    }
}

// Helper to read simple types
template <typename T>
T readValue(String path, T defaultValue) {
    T value = defaultValue;
    if (!InternalFS.exists(path.c_str())) {
        return defaultValue;
    }
    
    File file = InternalFS.open(path.c_str(), FILE_READ);
    if (file) {
        if (file.size() == sizeof(T)) {
            file.read((uint8_t*)&value, sizeof(T));
        }
        file.close();
    }
    return value;
}

String NrfPersistence::getFilePath(const char* key) {
    return "/" + _namespace + "/" + String(key);
}

void NrfPersistence::begin(const char* namespaceName, bool readOnly) {
    _namespace = String(namespaceName);
    InternalFS.begin();
    
    String dirPath = "/" + _namespace;
    if (!InternalFS.exists(dirPath.c_str())) {
        InternalFS.mkdir(dirPath.c_str());
    }
}

void NrfPersistence::end() {
    // No explicit close needed
}

void NrfPersistence::clear() {
    String dirPath = "/" + _namespace;
    File dir = InternalFS.open(dirPath.c_str());
    if (!dir || !dir.isDirectory()) {
        return;
    }

    File child = dir.openNextFile();
    while (child) {
        String childName = child.name();
        // Construct full path. child.name() is usually just the filename
        String fullPath = dirPath + "/" + childName;
        child.close(); // Close before removing
        
        InternalFS.remove(fullPath.c_str());
        
        child = dir.openNextFile();
    }
    dir.close();
}

void NrfPersistence::putInt(const char* key, int32_t value) {
    writeValue(getFilePath(key), value);
}

int32_t NrfPersistence::getInt(const char* key, int32_t defaultValue) {
    return readValue(getFilePath(key), defaultValue);
}

void NrfPersistence::putUInt(const char* key, uint32_t value) {
    writeValue(getFilePath(key), value);
}

uint32_t NrfPersistence::getUInt(const char* key, uint32_t defaultValue) {
    return readValue(getFilePath(key), defaultValue);
}

void NrfPersistence::putFloat(const char* key, float value) {
    writeValue(getFilePath(key), value);
}

float NrfPersistence::getFloat(const char* key, float defaultValue) {
    return readValue(getFilePath(key), defaultValue);
}

void NrfPersistence::putDouble(const char* key, double value) {
    writeValue(getFilePath(key), value);
}

double NrfPersistence::getDouble(const char* key, double defaultValue) {
    return readValue(getFilePath(key), defaultValue);
}

void NrfPersistence::putBool(const char* key, bool value) {
    writeValue(getFilePath(key), value);
}

bool NrfPersistence::getBool(const char* key, bool defaultValue) {
    return readValue(getFilePath(key), defaultValue);
}

void NrfPersistence::putUChar(const char* key, uint8_t value) {
    writeValue(getFilePath(key), value);
}

uint8_t NrfPersistence::getUChar(const char* key, uint8_t defaultValue) {
    return readValue(getFilePath(key), defaultValue);
}

void NrfPersistence::putBytes(const char* key, const void* value, size_t len) {
    File file = InternalFS.open(getFilePath(key).c_str(), FILE_WRITE);
    if (file) {
        file.write((const uint8_t*)value, len);
        file.close();
    }
}

size_t NrfPersistence::getBytes(const char* key, void* buf, size_t maxLen) {
    String path = getFilePath(key);
    if (!InternalFS.exists(path.c_str())) {
        return 0;
    }
    
    File file = InternalFS.open(path.c_str(), FILE_READ);
    size_t readLen = 0;
    if (file) {
        size_t fileSize = file.size();
        size_t toRead = (fileSize < maxLen) ? fileSize : maxLen;
        readLen = file.read((uint8_t*)buf, toRead);
        file.close();
    }
    return readLen;
}

size_t NrfPersistence::getBytesLength(const char* key) {
    String path = getFilePath(key);
    if (!InternalFS.exists(path.c_str())) {
        return 0;
    }
    
    File file = InternalFS.open(path.c_str(), FILE_READ);
    size_t len = 0;
    if (file) {
        len = file.size();
        file.close();
    }
    return len;
}
