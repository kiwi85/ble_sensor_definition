
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

// ========== PREDEFINED DEVICE PROFILES ==========

// Environmental Sensor Group Profile
inline DeviceProfile createEnvironmentalSensorProfile() {
    // Manufacturer data format for environmental sensors
    ManufacturerDataFormat mfgFormat(0xFFFF, "Environmental sensors data format");
    
    // Data format: [Battery(1)][Temp(2)][Humidity(2)][Pressure(4)][Altitude(2)]
    mfgFormat.dataFields = {
        DataFieldConfig("battery", 0, DataType::UINT8, 1.0f, "%"),
        DataFieldConfig("temperature", 1, DataType::INT16_BE, 0.01f, "°C"),
        DataFieldConfig("humidity", 3, DataType::UINT16_BE, 0.01f, "%"),
        DataFieldConfig("pressure", 5, DataType::UINT32_BE, 0.01f, "hPa"),
        DataFieldConfig("altitude", 9, DataType::INT16_BE, 0.1f, "m")
    };
    mfgFormat.totalLength = 11;
    
    DeviceProfile profile("Environmental_Sensors", "Environmental", mfgFormat);
    profile.serviceUuids.push_back(ServiceUUIDs::ENVIRONMENTAL);
    return profile;
}

// Air Quality Sensor Group Profile
inline DeviceProfile createAirQualitySensorProfile() {
    // Manufacturer data format for air quality sensors
    ManufacturerDataFormat mfgFormat(0xFFFF, "Air quality sensors data format");
    
    // Data format: [AQI(2)][TVOC(2)][eCO2(2)][Gas_resistance(4)]
    mfgFormat.dataFields = {
        DataFieldConfig("aqi", 0, DataType::UINT16_BE, 1.0f, "AQI"),
        DataFieldConfig("tvoc", 2, DataType::UINT16_BE, 1.0f, "ppb"),
        DataFieldConfig("co2", 4, DataType::UINT16_BE, 1.0f, "ppm"),
        DataFieldConfig("gas_resistance", 6, DataType::UINT32_BE, 1.0f, "Ohm")
    };
    mfgFormat.totalLength = 10;
    
    DeviceProfile profile("Air_Quality_Sensors", "AirQuality", mfgFormat);
    profile.serviceUuids.push_back(ServiceUUIDs::AIR_QUALITY);
    return profile;
}

// Motion Sensor Group Profile (IMU/Accelerometer/Gyroscope)
inline DeviceProfile createMotionSensorProfile() {
    // Manufacturer data format for motion sensors
    ManufacturerDataFormat mfgFormat(0xFFFF, "Motion sensors data format");
    
    // Data format: [AccelX(2)][AccelY(2)][AccelZ(2)][GyroX(2)][GyroY(2)][GyroZ(2)]
    mfgFormat.dataFields = {
        DataFieldConfig("accel_x", 0, DataType::INT16_BE, 0.001f, "g"),
        DataFieldConfig("accel_y", 2, DataType::INT16_BE, 0.001f, "g"),
        DataFieldConfig("accel_z", 4, DataType::INT16_BE, 0.001f, "g"),
        DataFieldConfig("gyro_x", 6, DataType::INT16_BE, 0.1f, "dps"),
        DataFieldConfig("gyro_y", 8, DataType::INT16_BE, 0.1f, "dps"),
        DataFieldConfig("gyro_z", 10, DataType::INT16_BE, 0.1f, "dps")
    };
    mfgFormat.totalLength = 12;
    
    DeviceProfile profile("Motion_Sensors", "Motion", mfgFormat);
    profile.serviceUuids.push_back(ServiceUUIDs::MOTION);
    return profile;
}

// Ambient Light/Color Sensor Group Profile
inline DeviceProfile createAmbientSensorProfile() {
    // Manufacturer data format for ambient sensors
    ManufacturerDataFormat mfgFormat(0xFFFF, "Ambient sensors data format");
    
    // Data format: [Brightness(2)][Red(1)][Green(1)][Blue(1)][White(1)]
    mfgFormat.dataFields = {
        DataFieldConfig("brightness", 0, DataType::UINT16_BE, 0.01f, "lux"),
        DataFieldConfig("red", 2, DataType::UINT8, 1.0f, ""),
        DataFieldConfig("green", 3, DataType::UINT8, 1.0f, ""),
        DataFieldConfig("blue", 4, DataType::UINT8, 1.0f, ""),
        DataFieldConfig("white", 5, DataType::UINT8, 1.0f, "")
    };
    mfgFormat.totalLength = 6;
    
    DeviceProfile profile("Ambient_Sensors", "Ambient", mfgFormat);
    profile.serviceUuids.push_back(ServiceUUIDs::AMBIENT);
    return profile;
}

// System/Power Management Sensor Group Profile
inline DeviceProfile createSystemSensorProfile() {
    // Manufacturer data format for system sensors
    ManufacturerDataFormat mfgFormat(0xFFFF, "System sensors data format");
    
    // Data format: [Battery_Level(1)][SOC(2)][Voltage(2)][Current(2)][Charging(1)]
    mfgFormat.dataFields = {
        DataFieldConfig("battery_level", 0, DataType::UINT8, 1.0f, "%"),
        DataFieldConfig("soc", 1, DataType::UINT16_BE, 0.01f, "%"),
        DataFieldConfig("voltage", 3, DataType::UINT16_BE, 0.001f, "V"),
        DataFieldConfig("current", 5, DataType::INT16_BE, 0.001f, "A"),
        DataFieldConfig("charging", 7, DataType::UINT8, 1.0f, "")
    };
    mfgFormat.totalLength = 8;
    
    DeviceProfile profile("System_Sensors", "System", mfgFormat);
    profile.serviceUuids.push_back(ServiceUUIDs::SYSTEM);
    return profile;
}

// Current Sensing Profile (SCT013, etc.)
inline DeviceProfile createCurrentSensorProfile() {
    // Manufacturer data format for current sensors
    ManufacturerDataFormat mfgFormat(0xFFFF, "Current sensors data format");
    
    // Data format: [RMS_Current(4)][Power(4)][Energy(4)]
    mfgFormat.dataFields = {
        DataFieldConfig("rms_current", 0, DataType::FLOAT_BE, 1.0f, "A"),
        DataFieldConfig("power", 4, DataType::FLOAT_BE, 1.0f, "W"),
        DataFieldConfig("energy", 8, DataType::FLOAT_BE, 1.0f, "Wh")
    };
    mfgFormat.totalLength = 12;
    
    DeviceProfile profile("Current_Sensors", "Current", mfgFormat);
    profile.serviceUuids.push_back(ServiceUUIDs::CURRENT);
    return profile;
}

// ========== DEVICE-SPECIFIC PROFILES ==========

// M5Stack with comprehensive sensor suite
inline DeviceProfile createM5StackComprehensiveProfile() {
    // Manufacturer data format combining multiple sensor types
    ManufacturerDataFormat mfgFormat(0xFFFF, "M5Stack comprehensive sensor format");
    
    // Data format: [Battery(1)][Temp(2)][Humidity(2)][Pressure(2)][AQI(2)][AccelMag(2)][Status(1)]
    mfgFormat.dataFields = {
        DataFieldConfig("battery", 0, DataType::UINT8, 1.0f, "%"),
        DataFieldConfig("temperature", 1, DataType::INT16_BE, 0.01f, "°C"),
        DataFieldConfig("humidity", 3, DataType::UINT16_BE, 0.01f, "%"),
        DataFieldConfig("pressure", 5, DataType::UINT16_BE, 0.1f, "hPa"),
        DataFieldConfig("air_quality", 7, DataType::UINT16_BE, 1.0f, "AQI"),
        DataFieldConfig("acceleration", 9, DataType::UINT16_BE, 0.001f, "g"),
        DataFieldConfig("status", 11, DataType::UINT8, 1.0f, "")
    };
    mfgFormat.totalLength = 12;
    
    DeviceProfile profile("M5Stack_Comprehensive", "M5Stack-Full", mfgFormat);
    profile.serviceUuids.push_back(ServiceUUIDs::ENVIRONMENTAL);
    profile.serviceUuids.push_back(ServiceUUIDs::AIR_QUALITY);
    profile.serviceUuids.push_back(ServiceUUIDs::MOTION);
    profile.serviceUuids.push_back(ServiceUUIDs::SYSTEM);
    return profile;
}

// ESP32 Development Board with common sensors
inline DeviceProfile createESP32DevBoardProfile() {
    // Manufacturer data format for ESP32 development board
    ManufacturerDataFormat mfgFormat(0xFFFF, "ESP32 development board sensor format");
    
    // Data format: [Temp(2)][Humidity(2)][Light(2)][Motion(1)][GPIO_Status(1)]
    mfgFormat.dataFields = {
        DataFieldConfig("temperature", 0, DataType::INT16_BE, 0.01f, "°C"),
        DataFieldConfig("humidity", 2, DataType::UINT16_BE, 0.01f, "%"),
        DataFieldConfig("brightness", 4, DataType::UINT16_BE, 0.1f, "lux"),
        DataFieldConfig("motion_detected", 6, DataType::UINT8, 1.0f, ""),
        DataFieldConfig("gpio_status", 7, DataType::UINT8, 1.0f, "")
    };
    mfgFormat.totalLength = 8;
    
    DeviceProfile profile("ESP32_DevBoard", "ESP32-Dev", mfgFormat);
    profile.serviceUuids.push_back(ServiceUUIDs::ENVIRONMENTAL);
    profile.serviceUuids.push_back(ServiceUUIDs::AMBIENT);
    return profile;
}

// Weather Station Profile (Outdoor environmental monitoring)
inline DeviceProfile createWeatherStationProfile() {
    // Manufacturer data format for weather station
    ManufacturerDataFormat mfgFormat(0xFFFF, "Weather station data format");
    
    // Data format: [Temp(2)][Humidity(2)][Pressure(4)][AQI(2)][UV(1)][Wind(1)]
    mfgFormat.dataFields = {
        DataFieldConfig("temperature", 0, DataType::INT16_BE, 0.01f, "°C"),
        DataFieldConfig("humidity", 2, DataType::UINT16_BE, 0.01f, "%"),
        DataFieldConfig("pressure", 4, DataType::UINT32_BE, 0.01f, "hPa"),
        DataFieldConfig("air_quality", 8, DataType::UINT16_BE, 1.0f, "AQI"),
        DataFieldConfig("uv_index", 10, DataType::UINT8, 0.1f, ""),
        DataFieldConfig("wind_speed", 11, DataType::UINT8, 0.1f, "m/s")
    };
    mfgFormat.totalLength = 12;
    
    DeviceProfile profile("Weather_Station", "WeatherStation", mfgFormat);
    profile.serviceUuids.push_back(ServiceUUIDs::ENVIRONMENTAL);
    profile.serviceUuids.push_back(ServiceUUIDs::AIR_QUALITY);
    return profile;
}

// M5Stack Environmental Sensor Profile (Original - kept for compatibility)
inline DeviceProfile createM5StackSensorProfile() {
    // Manufacturer data format (7 bytes of sensor data)
    ManufacturerDataFormat mfgFormat(0xFFFF, "M5Stack compact environmental sensor format");
    
    // Company ID: 0xFFFF (2 bytes) - automatically handled
    // Data format: [Battery(1)][Temp(2)][Humidity(2)][AirQuality(2)]
    mfgFormat.dataFields = {
        DataFieldConfig("battery", 0, DataType::UINT8, 1.0f, "%"),
        DataFieldConfig("temperature", 1, DataType::INT16_BE, 0.01f, "°C"),
        DataFieldConfig("humidity", 3, DataType::UINT16_BE, 0.01f, "%"),
        DataFieldConfig("air_quality", 5, DataType::UINT16_BE, 1.0f, "AQI")
    };
    mfgFormat.totalLength = 7; // Total data length (excluding company ID)
    
    DeviceProfile profile("Environmental Sensing", "M5Stack", mfgFormat);
    
    // Add service UUIDs for identification
    profile.serviceUuids.push_back(ServiceUUIDs::ENVIRONMENTAL); // Use new environmental service UUID
    
    return profile;
}

// Generic sensor profile (example for future expansion)
inline DeviceProfile createGenericSensorProfile() {
    ManufacturerDataFormat mfgFormat(0xFFFF, "Generic sensor format");
    
    // Simple format: [Value1(4 float)][Value2(4 float)]
    mfgFormat.dataFields = {
        DataFieldConfig("sensor1", 0, DataType::FLOAT_LE, 1.0f, ""),
        DataFieldConfig("sensor2", 4, DataType::FLOAT_LE, 1.0f, "")
    };
    mfgFormat.totalLength = 8;
    
    DeviceProfile profile("Generic_Sensor", "GenericDevice", mfgFormat);
    return profile;
}

// ========== PROFILE REGISTRY ==========

// Get all available profiles
inline std::vector<DeviceProfile> getAllProfiles() {
    return {
        // Original profiles (for compatibility)
        createM5StackSensorProfile(),
        createGenericSensorProfile(),
        
        // Sensor group profiles
        createEnvironmentalSensorProfile(),
        createAirQualitySensorProfile(),
        createMotionSensorProfile(),
        createAmbientSensorProfile(),
        createSystemSensorProfile(),
        createCurrentSensorProfile(),
        
        // Device-specific profiles
        createM5StackComprehensiveProfile(),
        createESP32DevBoardProfile(),
        createWeatherStationProfile()
    };
}

// Get profiles by sensor group
inline std::vector<DeviceProfile> getProfilesByGroup(SensorGroup group) {
    std::vector<DeviceProfile> profiles;
    
    switch (group) {
        case SensorGroup::ENVIRONMENTAL:
            profiles.push_back(createEnvironmentalSensorProfile());
            profiles.push_back(createWeatherStationProfile());
            break;
        case SensorGroup::AIR_QUALITY:
            profiles.push_back(createAirQualitySensorProfile());
            profiles.push_back(createWeatherStationProfile());
            break;
        case SensorGroup::MOTION:
            profiles.push_back(createMotionSensorProfile());
            break;
        case SensorGroup::AMBIENT:
            profiles.push_back(createAmbientSensorProfile());
            profiles.push_back(createESP32DevBoardProfile());
            break;
        case SensorGroup::SYSTEM:
            profiles.push_back(createSystemSensorProfile());
            profiles.push_back(createM5StackComprehensiveProfile());
            break;
        case SensorGroup::CURRENT:
            profiles.push_back(createCurrentSensorProfile());
            break;
    }
    
    return profiles;
}

// Get service UUID for sensor group
inline const char* getServiceUUIDForGroup(SensorGroup group) {
    switch (group) {
        case SensorGroup::ENVIRONMENTAL: return ServiceUUIDs::ENVIRONMENTAL;
        case SensorGroup::AIR_QUALITY: return ServiceUUIDs::AIR_QUALITY;
        case SensorGroup::MOTION: return ServiceUUIDs::MOTION;
        case SensorGroup::AMBIENT: return ServiceUUIDs::AMBIENT;
        case SensorGroup::SYSTEM: return ServiceUUIDs::SYSTEM;
        case SensorGroup::CURRENT: return ServiceUUIDs::CURRENT;
        default: return ServiceUUIDs::ENVIRONMENTAL;
    }
}

// Determine sensor group from sensor name
inline SensorGroup getSensorGroupFromName(const std::string& sensorName) {
    // Environmental sensors
    if (sensorName.find("bmp") != std::string::npos ||
        sensorName.find("hdc") != std::string::npos ||
        sensorName.find("sht") != std::string::npos ||
        sensorName.find("dht") != std::string::npos ||
        sensorName.find("aht") != std::string::npos ||
        sensorName.find("temperature") != std::string::npos ||
        sensorName.find("humidity") != std::string::npos ||
        sensorName.find("pressure") != std::string::npos) {
        return SensorGroup::ENVIRONMENTAL;
    }
    
    // Air quality sensors
    if (sensorName.find("ens") != std::string::npos ||
        sensorName.find("sgp") != std::string::npos ||
        sensorName.find("ccs") != std::string::npos ||
        sensorName.find("aqi") != std::string::npos ||
        sensorName.find("co2") != std::string::npos ||
        sensorName.find("tvoc") != std::string::npos) {
        return SensorGroup::AIR_QUALITY;
    }
    
    // Motion sensors
    if (sensorName.find("mpu") != std::string::npos ||
        sensorName.find("bmi") != std::string::npos ||
        sensorName.find("bmm") != std::string::npos ||
        sensorName.find("lsm") != std::string::npos ||
        sensorName.find("accel") != std::string::npos ||
        sensorName.find("gyro") != std::string::npos ||
        sensorName.find("magnet") != std::string::npos) {
        return SensorGroup::MOTION;
    }
    
    // Ambient sensors
    if (sensorName.find("veml") != std::string::npos ||
        sensorName.find("tsl") != std::string::npos ||
        sensorName.find("bh1") != std::string::npos ||
        sensorName.find("light") != std::string::npos ||
        sensorName.find("color") != std::string::npos ||
        sensorName.find("brightness") != std::string::npos) {
        return SensorGroup::AMBIENT;
    }
    
    // System sensors
    if (sensorName.find("bq") != std::string::npos ||
        sensorName.find("ip5306") != std::string::npos ||
        sensorName.find("battery") != std::string::npos ||
        sensorName.find("power") != std::string::npos ||
        sensorName.find("charging") != std::string::npos) {
        return SensorGroup::SYSTEM;
    }
    
    // Current sensors
    if (sensorName.find("sct") != std::string::npos ||
        sensorName.find("current") != std::string::npos) {
        return SensorGroup::CURRENT;
    }
    
    // Default to environmental
    return SensorGroup::ENVIRONMENTAL;
}

// Find profile by name
inline DeviceProfile* findProfileByName(const std::string& name, std::vector<DeviceProfile>& profiles) {
    for (auto& profile : profiles) {
        if (profile.profileName == name) {
            return &profile;
        }
    }
    return nullptr;
}

// Find profile by service UUID
inline DeviceProfile* findProfileByServiceUuid(const std::string& uuid, std::vector<DeviceProfile>& profiles) {
    for (auto& profile : profiles) {
        for (const auto& serviceUuid : profile.serviceUuids) {
            if (serviceUuid == uuid) {
                return &profile;
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
