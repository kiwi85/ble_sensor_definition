#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

// ========== SHARED BLE DEVICE PROFILE CONFIGURATION ==========
// This file can be used as a git submodule in both peripheral and central projects
// to ensure consistent data format definitions

namespace BLEProfiles {

// Sensor group categories for logical organization
enum class SensorGroup {
    ENVIRONMENTAL = 0,  // Temperature, Humidity, Pressure
    AIR_QUALITY = 1,    // Gas sensors, VOC, CO2, particles, AQI
    MOTION = 2,         // Accelerometer, Gyroscope, Magnetometer, IMU
    AMBIENT = 3,        // Light, UV, Color, Sound
    SYSTEM = 4,         // Battery, Power, Charging, System status
    CURRENT = 5         // Current sensing, power monitoring
};

// Service UUIDs for different sensor groups
namespace ServiceUUIDs {
    // Environmental sensors (temperature, humidity, pressure, altitude)
    const char* const ENVIRONMENTAL = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    
    // Air quality sensors (AQI, TVOC, eCO2, gas resistance)
    const char* const AIR_QUALITY = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
    
    // Motion sensors (accelerometer, gyroscope, magnetometer)
    const char* const MOTION = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
    
    // Ambient sensors (light, UV, sound)
    const char* const AMBIENT = "6E400004-B5A3-F393-E0A9-E50E24DCCA9E";
    
    // System sensors (battery, CPU temperature, memory, WiFi RSSI)
    const char* const SYSTEM = "6E400005-B5A3-F393-E0A9-E50E24DCCA9E";
    
    // Current sensors (voltage, current, power)
    const char* const CURRENT = "6E400006-B5A3-F393-E0A9-E50E24DCCA9E";
}

// Data field types supported
enum class DataType {
    UINT8,      // 1 byte unsigned integer
    INT8,       // 1 byte signed integer
    UINT16_LE,  // 2 bytes unsigned integer (little-endian)
    UINT16_BE,  // 2 bytes unsigned integer (big-endian)
    INT16_LE,   // 2 bytes signed integer (little-endian)
    INT16_BE,   // 2 bytes signed integer (big-endian)
    UINT32_LE,  // 4 bytes unsigned integer (little-endian)
    UINT32_BE,  // 4 bytes unsigned integer (big-endian)
    FLOAT_LE,   // 4 bytes IEEE 754 float (little-endian)
    FLOAT_BE    // 4 bytes IEEE 754 float (big-endian)
};

// Data field configuration - describes how to extract a single value from the data
struct DataFieldConfig {
    std::string sensorName;    // Friendly name (e.g., "battery", "temperature")
    uint8_t offset;            // Byte offset in the data packet
    DataType dataType;         // Data type and byte order
    float scale;               // Scale factor (value = raw * scale)
    std::string unit;          // Unit of measurement (e.g., "%", "°C", "ppm")
    
    DataFieldConfig(const std::string& name, uint8_t off, DataType type, float sc = 1.0f, const std::string& u = "")
        : sensorName(name), offset(off), dataType(type), scale(sc), unit(u) {}
};

// Manufacturer data format - used for connectionless broadcasting
struct ManufacturerDataFormat {
    uint16_t companyId;                          // BLE company ID (0xFFFF for testing)
    std::vector<DataFieldConfig> dataFields;     // Field configurations
    uint8_t totalLength;                         // Total data length (excluding company ID)
    std::string description;                     // Human-readable description
    
    // Default constructor
    ManufacturerDataFormat() : companyId(0xFFFF), totalLength(0), description("") {}
    
    ManufacturerDataFormat(uint16_t id, const std::string& desc)
        : companyId(id), totalLength(0), description(desc) {}
};

// Service data format - used for service-based advertising
struct ServiceDataFormat {
    std::string serviceUuid;                     // Service UUID
    std::vector<DataFieldConfig> dataFields;     // Field configurations
    uint8_t totalLength;                         // Total data length
    std::string description;                     // Human-readable description
    
    ServiceDataFormat(const std::string& uuid, const std::string& desc)
        : serviceUuid(uuid), totalLength(0), description(desc) {}
};

// Complete device profile - defines all data formats for a device
struct DeviceProfile {
    std::string profileName;                     // Unique profile name
    std::string deviceName;                      // Device name pattern to match
    std::vector<std::string> serviceUuids;       // Service UUIDs to identify device
    ManufacturerDataFormat manufacturerFormat;   // Manufacturer data format
    std::vector<ServiceDataFormat> serviceFormats; // Service data formats
    
    // Default constructor
    DeviceProfile() = default;
    
    DeviceProfile(const std::string& name, const std::string& device, 
                  const ManufacturerDataFormat& mfg)
        : profileName(name), deviceName(device), manufacturerFormat(mfg) {}
};

// ========== PREDEFINED DEVICE PROFILES ==========

// Environmental Sensor Group Profile
inline DeviceProfile createEnvironmentalSensorProfile() {
    // Manufacturer data format for environmental sensors
    ManufacturerDataFormat mfgFormat(0xFFFF, "Environmental sensors data format");
    
    // Data format: [Temp(2)][Humidity(2)][Pressure(4)][Altitude(2)]
    mfgFormat.dataFields = {
        DataFieldConfig("temperature", 0, DataType::INT16_BE, 0.01f, "°C"),
        DataFieldConfig("humidity", 2, DataType::UINT16_BE, 0.01f, "%"),
        DataFieldConfig("pressure", 4, DataType::UINT32_BE, 0.01f, "hPa"),
        DataFieldConfig("altitude", 8, DataType::INT16_BE, 0.1f, "m")
    };
    mfgFormat.totalLength = 10;
    
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
    
    DeviceProfile profile("M5Stack_Environmental", "M5Stack", mfgFormat);
    
    // Add service UUIDs for identification
    profile.serviceUuids.push_back("18dc799a-e8f3-4f75-83b5-d594b1c0e4a0"); // Custom Air Quality Service
    
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
    }
    return nullptr;
}

// Find profile by device name pattern
inline DeviceProfile* findProfileByDeviceName(const std::string& deviceName, std::vector<DeviceProfile>& profiles) {
    for (auto& profile : profiles) {
        if (deviceName.find(profile.deviceName) != std::string::npos) {
            return &profile;
        }
    }
    return nullptr;
}

// ========== DATA PARSING UTILITIES ==========

// Parse a value from raw data based on DataFieldConfig
inline float parseValue(const uint8_t* data, size_t dataLength, const DataFieldConfig& field) {
    if (field.offset >= dataLength) {
        return 0.0f; // Out of bounds
    }
    
    float rawValue = 0.0f;
    
    switch (field.dataType) {
        case DataType::UINT8:
            if (field.offset < dataLength) {
                rawValue = data[field.offset];
            }
            break;
            
        case DataType::INT8:
            if (field.offset < dataLength) {
                rawValue = (int8_t)data[field.offset];
            }
            break;
            
        case DataType::UINT16_LE:
            if (field.offset + 1 < dataLength) {
                rawValue = data[field.offset] | (data[field.offset + 1] << 8);
            }
            break;
            
        case DataType::UINT16_BE:
            if (field.offset + 1 < dataLength) {
                rawValue = (data[field.offset] << 8) | data[field.offset + 1];
            }
            break;
            
        case DataType::INT16_LE:
            if (field.offset + 1 < dataLength) {
                int16_t val = data[field.offset] | (data[field.offset + 1] << 8);
                rawValue = val;
            }
            break;
            
        case DataType::INT16_BE:
            if (field.offset + 1 < dataLength) {
                int16_t val = (data[field.offset] << 8) | data[field.offset + 1];
                rawValue = val;
            }
            break;
            
        case DataType::UINT32_LE:
            if (field.offset + 3 < dataLength) {
                rawValue = data[field.offset] | 
                          (data[field.offset + 1] << 8) | 
                          (data[field.offset + 2] << 16) | 
                          (data[field.offset + 3] << 24);
            }
            break;
            
        case DataType::UINT32_BE:
            if (field.offset + 3 < dataLength) {
                rawValue = (data[field.offset] << 24) | 
                          (data[field.offset + 1] << 16) | 
                          (data[field.offset + 2] << 8) | 
                          data[field.offset + 3];
            }
            break;
            
        case DataType::FLOAT_LE:
        case DataType::FLOAT_BE:
            if (field.offset + 3 < dataLength) {
                uint8_t floatBytes[4];
                if (field.dataType == DataType::FLOAT_LE) {
                    memcpy(floatBytes, &data[field.offset], 4);
                } else {
                    // Reverse bytes for big-endian
                    floatBytes[0] = data[field.offset + 3];
                    floatBytes[1] = data[field.offset + 2];
                    floatBytes[2] = data[field.offset + 1];
                    floatBytes[3] = data[field.offset];
                }
                memcpy(&rawValue, floatBytes, 4);
            }
            break;
    }
    
    return rawValue * field.scale;
}

// Parse all fields from manufacturer data
inline std::map<std::string, float> parseManufacturerData(const uint8_t* data, size_t dataLength, 
                                                           const ManufacturerDataFormat& format) {
    std::map<std::string, float> results;
    
    // Check minimum length (company ID + data)
    if (dataLength < 2) {
        return results; // Empty map = parsing failed
    }
    
    // Verify company ID matches (little-endian)
    uint16_t receivedCompanyId = data[0] | (data[1] << 8);
    if (receivedCompanyId != format.companyId) {
        return results; // Empty map = company ID mismatch
    }
    
    // Verify we have enough data for the expected format
    if (dataLength < (size_t)(2 + format.totalLength)) {
        return results; // Empty map = insufficient data
    }
    
    // Skip company ID (first 2 bytes)
    const uint8_t* sensorData = data + 2;
    size_t sensorDataLength = dataLength - 2;
    
    // Parse each field
    for (const auto& field : format.dataFields) {
        float value = parseValue(sensorData, sensorDataLength, field);
        results[field.sensorName] = value;
    }
    
    return results;
}

// Pack sensor values into manufacturer data format (for peripheral)
inline std::vector<uint8_t> packManufacturerData(const std::map<std::string, float>& values,
                                                  const ManufacturerDataFormat& format) {
    std::vector<uint8_t> data;
    
    // Add company ID (little-endian)
    data.push_back(format.companyId & 0xFF);
    data.push_back((format.companyId >> 8) & 0xFF);
    
    // Reserve space for all fields
    data.resize(2 + format.totalLength, 0);
    uint8_t* sensorData = &data[2];
    
    // Pack each field
    for (const auto& field : format.dataFields) {
        auto it = values.find(field.sensorName);
        if (it == values.end()) continue; // Skip missing values
        
        float scaledValue = it->second / field.scale;
        
        switch (field.dataType) {
            case DataType::UINT8:
                sensorData[field.offset] = (uint8_t)scaledValue;
                break;
                
            case DataType::INT8:
                sensorData[field.offset] = (int8_t)scaledValue;
                break;
                
            case DataType::UINT16_LE: {
                uint16_t val = (uint16_t)scaledValue;
                sensorData[field.offset] = val & 0xFF;
                sensorData[field.offset + 1] = (val >> 8) & 0xFF;
                break;
            }
                
            case DataType::UINT16_BE: {
                uint16_t val = (uint16_t)scaledValue;
                sensorData[field.offset] = (val >> 8) & 0xFF;
                sensorData[field.offset + 1] = val & 0xFF;
                break;
            }
                
            case DataType::INT16_LE: {
                int16_t val = (int16_t)scaledValue;
                sensorData[field.offset] = val & 0xFF;
                sensorData[field.offset + 1] = (val >> 8) & 0xFF;
                break;
            }
                
            case DataType::INT16_BE: {
                int16_t val = (int16_t)scaledValue;
                sensorData[field.offset] = (val >> 8) & 0xFF;
                sensorData[field.offset + 1] = val & 0xFF;
                break;
            }
                
            case DataType::FLOAT_LE: {
                float val = scaledValue;
                memcpy(&sensorData[field.offset], &val, 4);
                break;
            }
                
            case DataType::FLOAT_BE: {
                float val = scaledValue;
                uint8_t* floatBytes = (uint8_t*)&val;
                sensorData[field.offset] = floatBytes[3];
                sensorData[field.offset + 1] = floatBytes[2];
                sensorData[field.offset + 2] = floatBytes[1];
                sensorData[field.offset + 3] = floatBytes[0];
                break;
            }
                
            default:
                break;
        }
    }
    
    return data;
}

// ========== SENSOR GROUP UTILITIES ==========

// Pack sensor values into manufacturer data for a specific sensor group
inline std::vector<uint8_t> packSensorGroupData(const std::map<std::string, float>& values,
                                                 SensorGroup group) {
    DeviceProfile profile;
    
    switch (group) {
        case SensorGroup::ENVIRONMENTAL:
            profile = createEnvironmentalSensorProfile();
            break;
        case SensorGroup::AIR_QUALITY:
            profile = createAirQualitySensorProfile();
            break;
        case SensorGroup::MOTION:
            profile = createMotionSensorProfile();
            break;
        case SensorGroup::AMBIENT:
            profile = createAmbientSensorProfile();
            break;
        case SensorGroup::SYSTEM:
            profile = createSystemSensorProfile();
            break;
        case SensorGroup::CURRENT:
            profile = createCurrentSensorProfile();
            break;
        default:
            profile = createEnvironmentalSensorProfile();
            break;
    }
    
    return packManufacturerData(values, profile.manufacturerFormat);
}

// Parse sensor values from manufacturer data for a specific sensor group
inline std::map<std::string, float> parseSensorGroupData(const uint8_t* data, size_t dataLength,
                                                          SensorGroup group) {
    DeviceProfile profile;
    
    switch (group) {
        case SensorGroup::ENVIRONMENTAL:
            profile = createEnvironmentalSensorProfile();
            break;
        case SensorGroup::AIR_QUALITY:
            profile = createAirQualitySensorProfile();
            break;
        case SensorGroup::MOTION:
            profile = createMotionSensorProfile();
            break;
        case SensorGroup::AMBIENT:
            profile = createAmbientSensorProfile();
            break;
        case SensorGroup::SYSTEM:
            profile = createSystemSensorProfile();
            break;
        case SensorGroup::CURRENT:
            profile = createCurrentSensorProfile();
            break;
        default:
            profile = createEnvironmentalSensorProfile();
            break;
    }
    
    return parseManufacturerData(data, dataLength, profile.manufacturerFormat);
}

// Get human-readable name for sensor group
inline std::string getSensorGroupName(SensorGroup group) {
    switch (group) {
        case SensorGroup::ENVIRONMENTAL: return "Environmental";
        case SensorGroup::AIR_QUALITY: return "Air Quality";
        case SensorGroup::MOTION: return "Motion";
        case SensorGroup::AMBIENT: return "Ambient";
        case SensorGroup::SYSTEM: return "System";
        case SensorGroup::CURRENT: return "Current";
        default: return "Unknown";
    }
}

// Create multi-group sensor data packet
struct MultiGroupSensorData {
    SensorGroup group;
    std::vector<uint8_t> data;
    std::string serviceUuid;
};

// Pack multiple sensor groups into separate data packets
inline std::vector<MultiGroupSensorData> packMultiGroupSensorData(
    const std::map<SensorGroup, std::map<std::string, float>>& groupedValues) {
    
    std::vector<MultiGroupSensorData> packets;
    
    for (const auto& groupPair : groupedValues) {
        const SensorGroup& group = groupPair.first;
        const std::map<std::string, float>& values = groupPair.second;
        
        if (values.empty()) continue;
        
        MultiGroupSensorData packet;
        packet.group = group;
        packet.data = packSensorGroupData(values, group);
        packet.serviceUuid = getServiceUUIDForGroup(group);
        
        packets.push_back(packet);
    }
    
    return packets;
}

// Organize sensor values by group from a flat map
inline std::map<SensorGroup, std::map<std::string, float>> organizeSensorValuesByGroup(
    const std::map<std::string, std::map<std::string, float>>& sensorData) {
    
    std::map<SensorGroup, std::map<std::string, float>> groupedData;
    
    for (const auto& sensorPair : sensorData) {
        const std::string& sensorName = sensorPair.first;
        const std::map<std::string, float>& values = sensorPair.second;
        
        SensorGroup group = getSensorGroupFromName(sensorName);
        
        // Merge values into the appropriate group
        for (const auto& valuePair : values) {
            const std::string& key = valuePair.first;
            float value = valuePair.second;
            groupedData[group][key] = value;
        }
    }
    
    return groupedData;
}

} // namespace BLEProfiles
