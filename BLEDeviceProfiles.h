#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <cstring> // for memcpy

namespace BLEProfiles {

// -------------------------------------------------------------
// Sensor groups
// -------------------------------------------------------------
enum class SensorGroup {
    ENVIRONMENTAL = 0,
    AIR_QUALITY   = 1,
    MOTION        = 2,
    AMBIENT       = 3,
    SYSTEM        = 4,
    CURRENT       = 5,
    UNKNOWN       = 255
};

// -------------------------------------------------------------
// Data types
// -------------------------------------------------------------
enum class DataType {
    UINT8, INT8,
    UINT16_LE, UINT16_BE,
    INT16_LE,  INT16_BE,
    UINT32_LE, UINT32_BE,
    FLOAT_LE,  FLOAT_BE
};

// -------------------------------------------------------------
// Company IDs
// -------------------------------------------------------------
inline constexpr uint16_t COMPANY_ID_ENVIRONMENTAL = 0x1001;
inline constexpr uint16_t COMPANY_ID_AIR_QUALITY   = 0x1002;
inline constexpr uint16_t COMPANY_ID_MOTION        = 0x1003;
inline constexpr uint16_t COMPANY_ID_AMBIENT       = 0x1004;
inline constexpr uint16_t COMPANY_ID_SYSTEM        = 0x1005;
inline constexpr uint16_t COMPANY_ID_CURRENT       = 0x1006;

inline const std::map<SensorGroup, uint16_t> SENSOR_COMPANY_ID_MAP = {
    { SensorGroup::ENVIRONMENTAL, COMPANY_ID_ENVIRONMENTAL },
    { SensorGroup::AIR_QUALITY,   COMPANY_ID_AIR_QUALITY },
    { SensorGroup::MOTION,        COMPANY_ID_MOTION },
    { SensorGroup::AMBIENT,       COMPANY_ID_AMBIENT },
    { SensorGroup::SYSTEM,        COMPANY_ID_SYSTEM },
    { SensorGroup::CURRENT,       COMPANY_ID_CURRENT }
};

inline const std::map<uint16_t, SensorGroup> COMPANY_ID_TO_GROUP = {
    { COMPANY_ID_ENVIRONMENTAL, SensorGroup::ENVIRONMENTAL },
    { COMPANY_ID_AIR_QUALITY,   SensorGroup::AIR_QUALITY },
    { COMPANY_ID_MOTION,        SensorGroup::MOTION },
    { COMPANY_ID_AMBIENT,       SensorGroup::AMBIENT },
    { COMPANY_ID_SYSTEM,        SensorGroup::SYSTEM },
    { COMPANY_ID_CURRENT,       SensorGroup::CURRENT }
};

inline const std::map<std::string, BLEProfiles::SensorGroup> PROFILE_NAME_TO_GROUP = {
    { "EnvironmentalSensor", BLEProfiles::SensorGroup::ENVIRONMENTAL },
    { "AirQualitySensor",    BLEProfiles::SensorGroup::AIR_QUALITY },
    { "MotionSensor",        BLEProfiles::SensorGroup::MOTION },
    { "AmbientSensor",       BLEProfiles::SensorGroup::AMBIENT },
    { "SystemSensor",        BLEProfiles::SensorGroup::SYSTEM },
    { "CurrentSensor",       BLEProfiles::SensorGroup::CURRENT }
};

// -------------------------------------------------------------
// Data structures
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

struct ManufacturerDataFormat {
    uint16_t companyId;
    std::vector<DataFieldConfig> dataFields;
    uint8_t totalLength;
    std::string description;

    ManufacturerDataFormat() : companyId(0), totalLength(0), description("") {}
    ManufacturerDataFormat(uint16_t id, const std::string& desc)
        : companyId(id), totalLength(0), description(desc) {}
};

struct DeviceProfile {
    std::string profileName;
    std::string deviceName;
    ManufacturerDataFormat manufacturerFormat;
};

// -------------------------------------------------------------
// Profile creation helper
// -------------------------------------------------------------
inline DeviceProfile createEnvironmentalProfile() {
    ManufacturerDataFormat mfg(COMPANY_ID_ENVIRONMENTAL, "Environmental Manufacturer Data");
    mfg.dataFields = {
        {"Temperature", 0, DataType::INT16_LE, 0.01f, "°C"},
        {"Humidity",    2, DataType::UINT16_LE, 0.01f, "%"},
        {"Pressure",    4, DataType::UINT32_LE, 0.001f, "hPa"},
        {"Battery",     8, DataType::UINT8,     1.0f, "%"}
    };
    mfg.totalLength = 9;
    return { "EnvironmentalSensor", "EnviroSensor-X", mfg };
}

// -------------------------------------------------------------
// Utility to get size of DataType in bytes
// -------------------------------------------------------------
inline size_t dataTypeSize(DataType dt) {
    switch(dt) {
        case DataType::UINT8:
        case DataType::INT8:    return 1;
        case DataType::UINT16_LE:
        case DataType::UINT16_BE:
        case DataType::INT16_LE:
        case DataType::INT16_BE: return 2;
        case DataType::UINT32_LE:
        case DataType::UINT32_BE:
        case DataType::FLOAT_LE:
        case DataType::FLOAT_BE: return 4;
    }
    return 0;
}

// -------------------------------------------------------------
// Manufacturer data packing helpers
// -------------------------------------------------------------
inline std::vector<uint8_t> packManufacturerData(
    const std::map<std::string, float>& sensorValues,
    const ManufacturerDataFormat& format)
{
    std::vector<uint8_t> data(2 + format.totalLength, 0);

    // First two bytes: company ID (little endian)
    data[0] = static_cast<uint8_t>(format.companyId & 0xFF);
    data[1] = static_cast<uint8_t>((format.companyId >> 8) & 0xFF);

    for (const auto& field : format.dataFields) {
        auto it = sensorValues.find(field.sensorName);
        if (it == sensorValues.end()) continue;

        float val = it->second / (field.scale != 0.0f ? field.scale : 1.0f);

        switch (field.dataType) {
            case DataType::UINT8:
                data[2 + field.offset] = static_cast<uint8_t>(val);
                break;
            case DataType::INT8:
                data[2 + field.offset] = static_cast<int8_t>(val);
                break;
            case DataType::UINT16_LE: {
                uint16_t v = static_cast<uint16_t>(val);
                data[2 + field.offset]     = v & 0xFF;
                data[2 + field.offset + 1] = (v >> 8) & 0xFF;
                break;
            }
            case DataType::INT16_LE: {
                int16_t v = static_cast<int16_t>(val);
                data[2 + field.offset]     = v & 0xFF;
                data[2 + field.offset + 1] = (v >> 8) & 0xFF;
                break;
            }
            case DataType::UINT32_LE: {
                uint32_t v = static_cast<uint32_t>(val);
                data[2 + field.offset]     = v & 0xFF;
                data[2 + field.offset + 1] = (v >> 8) & 0xFF;
                data[2 + field.offset + 2] = (v >> 16) & 0xFF;
                data[2 + field.offset + 3] = (v >> 24) & 0xFF;
                break;
            }
            case DataType::FLOAT_LE: {
                float fval = val;
                memcpy(&data[2 + field.offset], &fval, sizeof(float));
                break;
            }
            default:
                break; // unsupported type
        }
    }

    return data;
}

// -------------------------------------------------------------
// Pack data for a sensor group using the associated device profile
// -------------------------------------------------------------
inline std::vector<uint8_t> packSensorGroupData(
    const std::map<std::string, float>& sensorValues,
    SensorGroup /*group*/,
    const DeviceProfile& profile)
{
    return packManufacturerData(sensorValues, profile.manufacturerFormat);
}

// -------------------------------------------------------------
// Parse manufacturer data (handles company ID prefix)
// -------------------------------------------------------------
inline std::map<std::string,float> parseManufacturerData(
    const uint8_t* data,
    size_t len,
    const ManufacturerDataFormat& format
) {
    std::map<std::string,float> values;
    if (!data || len < 2) return values;

    // Skip first two bytes (company ID)
    const uint8_t* payload = data + 2;
    size_t payloadLen = len - 2;

    for (const auto& field : format.dataFields) {
        size_t fieldLen = dataTypeSize(field.dataType);
        if (field.offset + fieldLen > payloadLen) continue;

        float parsedValue = 0.0f;

        switch (field.dataType) {
            case DataType::UINT8:   parsedValue = static_cast<float>(payload[field.offset]); break;
            case DataType::INT8:    parsedValue = static_cast<float>(static_cast<int8_t>(payload[field.offset])); break;
            case DataType::UINT16_LE:
                parsedValue = static_cast<float>(payload[field.offset] | (payload[field.offset+1] << 8));
                break;
            case DataType::INT16_LE:
                parsedValue = static_cast<float>(static_cast<int16_t>(payload[field.offset] | (payload[field.offset+1] << 8)));
                break;
            case DataType::UINT32_LE:
                parsedValue = static_cast<float>(
                    payload[field.offset] |
                    (payload[field.offset+1] << 8) |
                    (payload[field.offset+2] << 16) |
                    (payload[field.offset+3] << 24)
                );
                break;
            case DataType::FLOAT_LE:
                {
                    float val;
                    memcpy(&val, payload + field.offset, sizeof(float));
                    parsedValue = val;
                }
                break;
            default: break;
        }

        if (field.scale != 0.0f) parsedValue *= field.scale;
        values[field.sensorName] = parsedValue;
    }

    return values;
}

// -------------------------------------------------------------
// Utility: convert SensorGroup enum to string
// -------------------------------------------------------------
inline const char* getSensorGroupName(SensorGroup group) {
    switch (group) {
        case SensorGroup::ENVIRONMENTAL: return "Environmental";
        case SensorGroup::AIR_QUALITY:   return "AirQuality";
        case SensorGroup::MOTION:        return "Motion";
        case SensorGroup::AMBIENT:       return "Ambient";
        case SensorGroup::SYSTEM:        return "System";
        case SensorGroup::CURRENT:       return "Current";
        default: return "Unknown";
    }
}

// -------------------------------------------------------------
// Lookup utilities
// -------------------------------------------------------------
inline uint16_t getCompanyIdForGroup(SensorGroup group) {
    auto it = SENSOR_COMPANY_ID_MAP.find(group);
    if (it != SENSOR_COMPANY_ID_MAP.end()) return it->second;
    return 0xFFFF;
}

inline SensorGroup getGroupForCompanyId(uint16_t companyId) {
    auto it = COMPANY_ID_TO_GROUP.find(companyId);
    if (it != COMPANY_ID_TO_GROUP.end()) return it->second;
    return SensorGroup::UNKNOWN;
}

// -------------------------------------------------------------
// Get all device profiles
// -------------------------------------------------------------
inline std::vector<DeviceProfile> getAllProfiles() {
    return { createEnvironmentalProfile() };
}

} // namespace BLEProfiles
