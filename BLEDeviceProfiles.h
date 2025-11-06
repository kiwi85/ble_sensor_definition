#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <cstring>

namespace BLEProfiles {
static constexpr uint16_t COMPANY_ID = 0x1234; // Example Company Identifier (replace with actual)
// -------------------------------------------------------------
// Sensor group categories
// -------------------------------------------------------------
enum class SensorGroup {
    ENVIRONMENTAL = 0,
    AIR_QUALITY   = 1,
    MOTION        = 2,
    AMBIENT       = 3,
    SYSTEM        = 4,
    CURRENT       = 5
};

// -------------------------------------------------------------
// Service UUIDs
// -------------------------------------------------------------
namespace ServiceUUIDs {
    const char* const ENVIRONMENTAL = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    const char* const AIR_QUALITY   = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
    const char* const MOTION        = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
    const char* const AMBIENT       = "6E400004-B5A3-F393-E0A9-E50E24DCCA9E";
    const char* const SYSTEM        = "6E400005-B5A3-F393-E0A9-E50E24DCCA9E";
    const char* const CURRENT       = "6E400006-B5A3-F393-E0A9-E50E24DCCA9E";
}

// -------------------------------------------------------------
// DataType enum
// -------------------------------------------------------------
enum class DataType {
    UINT8, INT8,
    UINT16_LE, UINT16_BE,
    INT16_LE,  INT16_BE,
    UINT32_LE, UINT32_BE,
    FLOAT_LE,  FLOAT_BE
};

// -------------------------------------------------------------
// DataFieldConfig
// -------------------------------------------------------------
struct DataFieldConfig {
    std::string sensorName;
    uint8_t offset;
    DataType dataType;
    float scale;
    std::string unit;

    DataFieldConfig(const std::string& name, uint8_t off, DataType type,
                    float sc = 1.0f, const std::string& u = "")
        : sensorName(name), offset(off), dataType(type), scale(sc), unit(u) {}
};

// -------------------------------------------------------------
// ManufacturerDataFormat
// -------------------------------------------------------------
struct ManufacturerDataFormat {
    uint16_t companyId;
    std::vector<DataFieldConfig> dataFields;
    uint8_t totalLength;
    std::string description;

    ManufacturerDataFormat() : companyId(COMPANY_ID), totalLength(0), description("") {}
    ManufacturerDataFormat(uint16_t id, const std::string& desc)
        : companyId(id), totalLength(0), description(desc) {}
};

// -------------------------------------------------------------
// ServiceDataFormat
// -------------------------------------------------------------
struct ServiceDataFormat {
    std::string serviceUuid;
    std::vector<DataFieldConfig> dataFields;
    uint8_t totalLength;
    std::string description;

    ServiceDataFormat(const std::string& uuid, const std::string& desc)
        : serviceUuid(uuid), totalLength(0), description(desc) {}
};

// -------------------------------------------------------------
// DeviceProfile
// -------------------------------------------------------------
struct DeviceProfile {
    std::string profileName;
    std::string deviceName;
    std::vector<std::string> serviceUuids;
    ManufacturerDataFormat manufacturerFormat;
    std::vector<ServiceDataFormat> serviceFormats;

    DeviceProfile() = default;
    DeviceProfile(const std::string& name, const std::string& device,
                  const ManufacturerDataFormat& mfg)
        : profileName(name), deviceName(device), manufacturerFormat(mfg) {}
};

// -------------------------------------------------------------
// Utility: parseValue
// -------------------------------------------------------------
inline float parseValue(const uint8_t* data, size_t dataLength,
                        const DataFieldConfig& field)
{
    if (field.offset >= dataLength) return 0.0f;
    float rawValue = 0.0f;

    switch (field.dataType) {
        case DataType::UINT8: rawValue = data[field.offset]; break;
        case DataType::INT8: rawValue = static_cast<int8_t>(data[field.offset]); break;

        case DataType::UINT16_LE:
            rawValue = data[field.offset] | (data[field.offset + 1] << 8); break;
        case DataType::UINT16_BE:
            rawValue = (data[field.offset] << 8) | data[field.offset + 1]; break;

        case DataType::INT16_LE: {
            int16_t v = data[field.offset] | (data[field.offset + 1] << 8);
            rawValue = v; break;
        }
        case DataType::INT16_BE: {
            int16_t v = (data[field.offset] << 8) | data[field.offset + 1];
            rawValue = v; break;
        }

        case DataType::UINT32_LE:
            rawValue = data[field.offset] |
                       (data[field.offset + 1] << 8) |
                       (data[field.offset + 2] << 16) |
                       (data[field.offset + 3] << 24);
            break;

        case DataType::UINT32_BE:
            rawValue = (data[field.offset] << 24) |
                       (data[field.offset + 1] << 16) |
                       (data[field.offset + 2] << 8) |
                       data[field.offset + 3];
            break;

        case DataType::FLOAT_LE:
        case DataType::FLOAT_BE: {
            uint8_t bytes[4];
            if (field.dataType == DataType::FLOAT_LE)
                memcpy(bytes, &data[field.offset], 4);
            else {
                bytes[0] = data[field.offset + 3];
                bytes[1] = data[field.offset + 2];
                bytes[2] = data[field.offset + 1];
                bytes[3] = data[field.offset];
            }
            memcpy(&rawValue, bytes, 4);
            break;
        }
    }

    return rawValue * field.scale;
}

// -------------------------------------------------------------
// Parse manufacturer and service data
// -------------------------------------------------------------
inline std::map<std::string, float> parseManufacturerData(
    const uint8_t* data, size_t dataLength,
    const ManufacturerDataFormat& format)
{
    std::map<std::string, float> results;
    if (dataLength < 2) return results;

    uint16_t companyId = data[0] | (data[1] << 8);
    if (companyId != format.companyId) return results;
    if (dataLength < static_cast<size_t>(2 + format.totalLength)) return results;

    const uint8_t* payload = data + 2;
    size_t len = dataLength - 2;

    for (const auto& field : format.dataFields)
        results[field.sensorName] = parseValue(payload, len, field);

    return results;
}

inline std::map<std::string, float> parseServiceData(
    const uint8_t* data, size_t dataLength,
    const ServiceDataFormat& format)
{
    std::map<std::string, float> results;
    if (dataLength < format.totalLength) return results;

    for (const auto& field : format.dataFields)
        results[field.sensorName] = parseValue(data, dataLength, field);

    return results;
}

// -------------------------------------------------------------
// Profile creation helpers
// -------------------------------------------------------------
inline DeviceProfile createEnviromentalProfile() {
    ManufacturerDataFormat mfg(COMPANY_ID, "Enviromental Manufacturer Data");
    mfg.dataFields = {
        {"Temperature", 0, DataType::INT16_LE, 0.01f, "°C"},
        {"Humidity",    2, DataType::UINT16_LE, 0.01f, "%"},
        {"Pressure",    4, DataType::UINT32_LE, 0.001f, "hPa"}
    };
    mfg.totalLength = 8;

    ServiceDataFormat service(ServiceUUIDs::ENVIRONMENTAL, "Environmental Sensor Data");
    service.dataFields = mfg.dataFields;
    service.totalLength = 8;

    DeviceProfile profile("EnvironmentalSensor", "EnviroSensor-X", mfg);
    profile.serviceUuids = { ServiceUUIDs::ENVIRONMENTAL };
    profile.serviceFormats = { service };
    return profile;
}

// alias for your existing code
inline DeviceProfile createEnvironmentalSensorProfile() {
    return createEnviromentalProfile();
}

// -------------------------------------------------------------
// Utility mapping functions expected by main.cpp
// -------------------------------------------------------------
inline std::string getSensorGroupName(SensorGroup group) {
    switch (group) {
        case SensorGroup::ENVIRONMENTAL: return "Environmental";
        case SensorGroup::AIR_QUALITY:   return "AirQuality";
        case SensorGroup::MOTION:        return "Motion";
        case SensorGroup::AMBIENT:       return "Ambient";
        case SensorGroup::SYSTEM:        return "System";
        case SensorGroup::CURRENT:       return "Current";
        default:                         return "Unknown";
    }
}

inline const char* getServiceUUIDForGroup(SensorGroup group) {
    switch (group) {
        case SensorGroup::ENVIRONMENTAL: return ServiceUUIDs::ENVIRONMENTAL;
        case SensorGroup::AIR_QUALITY:   return ServiceUUIDs::AIR_QUALITY;
        case SensorGroup::MOTION:        return ServiceUUIDs::MOTION;
        case SensorGroup::AMBIENT:       return ServiceUUIDs::AMBIENT;
        case SensorGroup::SYSTEM:        return ServiceUUIDs::SYSTEM;
        case SensorGroup::CURRENT:       return ServiceUUIDs::CURRENT;
        default:                         return "";
    }
}

// -------------------------------------------------------------
// Data packing helpers
// -------------------------------------------------------------
inline std::vector<uint8_t> packManufacturerData(
    const std::map<std::string, float>& sensorValues,
    const ManufacturerDataFormat& format)
{
    std::vector<uint8_t> data(2 + format.totalLength, 0);
    data[0] = static_cast<uint8_t>(format.companyId & 0xFF);
    data[1] = static_cast<uint8_t>(format.companyId >> 8);

    for (const auto& field : format.dataFields) {
        auto it = sensorValues.find(field.sensorName);
        if (it == sensorValues.end()) continue;
        float val = it->second / field.scale;

        switch (field.dataType) {
            case DataType::INT16_LE: {
                int16_t v = static_cast<int16_t>(val);
                data[2 + field.offset]     = v & 0xFF;
                data[2 + field.offset + 1] = (v >> 8) & 0xFF;
                break;
            }
            default: break;
        }
    }
    return data;
}

inline std::vector<uint8_t> packSensorGroupData(
    const std::map<std::string, float>& sensorValues,
    SensorGroup group)
{
    DeviceProfile profile;
    if (group == SensorGroup::ENVIRONMENTAL)
        profile = createEnviromentalProfile();
    else
        return {};

    return packManufacturerData(sensorValues, profile.manufacturerFormat);
}

// -------------------------------------------------------------
// getAllProfiles
// -------------------------------------------------------------
inline std::vector<DeviceProfile> getAllProfiles() {
    return { createEnviromentalProfile() };
}

} // namespace BLEProfiles
