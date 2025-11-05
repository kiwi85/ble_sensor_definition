#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <cstring>
#include <algorithm>

#ifdef ARDUINO_JSON_VERSION
#include <ArduinoJson.h>
#endif

namespace BLEProfiles {

// ─────────────────────────────────────────────
// 1. Data types and configuration structures
// ─────────────────────────────────────────────

enum class DataType {
    UINT8,
    INT8,
    UINT16_BE,
    INT16_BE,
    UINT32_BE,
    INT32_BE,
    FLOAT32
};

struct DataFieldConfig {
    std::string name;
    uint8_t offset;
    DataType type;
    float scale;
    std::string unit;
};

struct SimpleDeviceProfile {
    std::string name;
    std::string serviceUuid;
    uint16_t companyId;
    std::vector<DataFieldConfig> fields;
};

// ─────────────────────────────────────────────
// 2. Real static UUIDs (v4, unique per profile)
// ─────────────────────────────────────────────
// Generated with RFC 4122 random UUIDs.

static const char* UUID_WEATHER_STATION =  "e3b5f4c6-6b4f-4a1f-8b22-962a315a9c9b";
static const char* UUID_AIR_QUALITY     =  "5e4a7bb8-3a1e-4a3a-b3a4-38b28a09118d";
static const char* UUID_POWER_MONITOR   =  "cb8b69d4-1b8d-4c83-a0b7-4d857f5f9ac4";
static const char* UUID_ENVIRONMENTAL   =  "2b83f732-daf7-41a9-a214-f6b726e2927d";
static const char* UUID_M5STACK_SENSOR  =  "18dc799a-e8f3-4f75-83b5-d594b1c0e4a0";  // Your M5Stack device

// ─────────────────────────────────────────────
// 3. Known device profiles (manual layout)
// ─────────────────────────────────────────────

inline const std::vector<SimpleDeviceProfile>& getProfiles() {
    static const std::vector<SimpleDeviceProfile> profiles = {

        // ───────────── Weather Station ─────────────
        {
            "WeatherStation", UUID_WEATHER_STATION, 0xFFFF, {
                { "battery",     0, DataType::UINT8,      1.0f,  "%"   },
                { "temperature", 1, DataType::INT16_BE,   0.01f, "°C"  },
                { "humidity",    3, DataType::UINT16_BE,  0.01f, "%"   },
                { "pressure",    5, DataType::UINT32_BE,  0.01f, "hPa" },
            }
        },

        // ───────────── Air Quality Node ─────────────
        {
            "AirQualityNode", UUID_AIR_QUALITY, 0xFFFF, {
                { "battery",  0, DataType::UINT8,     1.0f, "%"   },
                { "aqi",      1, DataType::UINT16_BE, 1.0f, "AQI" },
                { "tvoc",     3, DataType::UINT16_BE, 1.0f, "ppb" },
                { "co2",      5, DataType::UINT16_BE, 1.0f, "ppm" },
            }
        },

        // ───────────── Power Monitor ─────────────
        {
            "PowerMonitor", UUID_POWER_MONITOR, 0xFFFF, {
                { "battery",  0, DataType::UINT8,     1.0f,  "%"  },
                { "voltage",  1, DataType::UINT16_BE, 0.01f, "V"  },
                { "current",  3, DataType::INT16_BE,  0.001f,"A"  },
                { "power",    5, DataType::INT32_BE,  0.001f,"W"  },
            }
        },

        // ───────────── Environmental Node ─────────────
        {
            "EnvironmentalNode", UUID_ENVIRONMENTAL, 0xFFFF, {
                { "battery",     0, DataType::UINT8,      1.0f,  "%"   },
                { "temperature", 1, DataType::INT16_BE,   0.01f, "°C"  },
                { "humidity",    3, DataType::UINT16_BE,  0.01f, "%"   },
                { "pressure",    5, DataType::UINT32_BE,  0.01f, "hPa" },
            }
        },

        // ───────────── M5Stack Sensor ─────────────
        {
            "M5StackSensor", UUID_M5STACK_SENSOR, 0xFFFF, {
                { "battery",     0, DataType::UINT8,      1.0f,  "%"   },
                { "temperature", 1, DataType::INT16_BE,   0.01f, "°C"  },
                { "humidity",    3, DataType::UINT16_BE,  0.01f, "%"   },
                { "pressure",    5, DataType::UINT32_BE,  0.01f, "hPa" },
            }
        }
    };
    return profiles;
}

// ─────────────────────────────────────────────
// 4. Utility: profile lookup by UUID or name
// ─────────────────────────────────────────────

inline const SimpleDeviceProfile* findProfileByUuid(const std::string& uuid) {
    const auto& profiles = getProfiles();
    auto it = std::find_if(profiles.begin(), profiles.end(),
                           [&](const SimpleDeviceProfile& p){ return p.serviceUuid == uuid; });
    return (it != profiles.end()) ? &(*it) : nullptr;
}

inline const SimpleDeviceProfile* findProfileByName(const std::string& name) {
    const auto& profiles = getProfiles();
    auto it = std::find_if(profiles.begin(), profiles.end(),
                           [&](const SimpleDeviceProfile& p){ return p.name == name; });
    return (it != profiles.end()) ? &(*it) : nullptr;
}

// ─────────────────────────────────────────────
// 5. Binary parsing and packing logic
// ─────────────────────────────────────────────

inline float parseField(const uint8_t* data, DataType t) {
    switch (t) {
        case DataType::UINT8:     return data[0];
        case DataType::INT8:      return (int8_t)data[0];
        case DataType::UINT16_BE: return (data[0] << 8) | data[1];
        case DataType::INT16_BE:  return (int16_t)((data[0] << 8) | data[1]);
        case DataType::UINT32_BE: return ((uint32_t)data[0] << 24) |
                                            ((uint32_t)data[1] << 16) |
                                            ((uint32_t)data[2] << 8) |
                                            ((uint32_t)data[3]);
        case DataType::INT32_BE:  return (int32_t)(((uint32_t)data[0] << 24) |
                                                   ((uint32_t)data[1] << 16) |
                                                   ((uint32_t)data[2] << 8) |
                                                   ((uint32_t)data[3]));
        case DataType::FLOAT32: {
            float f;
            std::memcpy(&f, data, 4);
            return f;
        }
    }
    return 0;
}

inline std::map<std::string, float> parseManufacturerData(
        const uint8_t* buf, size_t len,
        const SimpleDeviceProfile& profile) {
    std::map<std::string, float> out;
    for (auto& f : profile.fields) {
        if (f.offset + 4 > len) continue;
        float val = parseField(buf + f.offset, f.type) * f.scale;
        out[f.name] = val;
    }
    return out;
}

inline std::vector<uint8_t> packManufacturerData(
        const std::map<std::string, float>& values,
        const SimpleDeviceProfile& profile) {
    size_t payloadSize = 0;
    for (auto& f : profile.fields)
        payloadSize = std::max(payloadSize, (size_t)f.offset + 4);
    std::vector<uint8_t> data(payloadSize, 0);

    for (auto& f : profile.fields) {
        float v = values.count(f.name) ? values.at(f.name) / f.scale : 0;
        switch (f.type) {
            case DataType::UINT8:  data[f.offset] = (uint8_t)v; break;
            case DataType::INT8:   data[f.offset] = (int8_t)v; break;
            case DataType::UINT16_BE:
            case DataType::INT16_BE: {
                int16_t iv = (int16_t)v;
                data[f.offset]   = iv >> 8;
                data[f.offset+1] = iv & 0xFF;
                break;
            }
            case DataType::UINT32_BE:
            case DataType::INT32_BE: {
                int32_t iv = (int32_t)v;
                data[f.offset]   = (iv >> 24) & 0xFF;
                data[f.offset+1] = (iv >> 16) & 0xFF;
                data[f.offset+2] = (iv >> 8)  & 0xFF;
                data[f.offset+3] = iv & 0xFF;
                break;
            }
            case DataType::FLOAT32:
                std::memcpy(&data[f.offset], &v, 4);
                break;
        }
    }
    return data;
}

// ─────────────────────────────────────────────
// 6. Optional JSON helpers (ArduinoJson integration)
// ─────────────────────────────────────────────
#ifdef ARDUINO_JSON_VERSION

inline std::map<std::string, float> jsonToMap(const JsonObject& json) {
    std::map<std::string, float> values;
    for (auto kv : json) {
        if (kv.value().is<float>()) {
            values[kv.key().c_str()] = kv.value().as<float>();
        } else if (kv.value().is<int>()) {
            values[kv.key().c_str()] = (float)kv.value().as<int>();
        }
    }
    return values;
}

inline void mapToJson(const std::map<std::string, float>& values, JsonObject& out) {
    for (auto& kv : values) {
        out[kv.first] = kv.second;
    }
}

#endif // ARDUINO_JSON_VERSION

} // namespace BLEProfiles
