#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

// ========== SHARED BLE DEVICE PROFILE CONFIGURATION ==========
// This file can be used as a git submodule in both peripheral and central projects
// to ensure consistent data format definitions

namespace BLEProfiles {

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
    
    DeviceProfile(const std::string& name, const std::string& device, 
                  const ManufacturerDataFormat& mfg)
        : profileName(name), deviceName(device), manufacturerFormat(mfg) {}
};

// ========== PREDEFINED DEVICE PROFILES ==========

// M5Stack Environmental Sensor Profile
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
        createM5StackSensorProfile(),
        createGenericSensorProfile()
        // Add more profiles here as needed
    };
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

} // namespace BLEProfiles
