#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>

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
    CURRENT       = 5
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
// Company IDs (one per sensor group, replaces UUIDs)
// -------------------------------------------------------------
inline constexpr uint16_t COMPANY_ID_ENVIRONMENTAL = 0x1001;
inline constexpr uint16_t COMPANY_ID_AIR_QUALITY   = 0x1002;
inline constexpr uint16_t COMPANY_ID_MOTION        = 0x1003;
inline constexpr uint16_t COMPANY_ID_AMBIENT       = 0x1004;
inline constexpr uint16_t COMPANY_ID_SYSTEM        = 0x1005;
inline constexpr uint16_t COMPANY_ID_CURRENT       = 0x1006;

// -------------------------------------------------------------
// Mapping structure for iteration
// -------------------------------------------------------------
inline const std::map<SensorGroup, uint16_t> SENSOR_COMPANY_ID_MAP = {
    { SensorGroup::ENVIRONMENTAL, COMPANY_ID_ENVIRONMENTAL },
    { SensorGroup::AIR_QUALITY,   COMPANY_ID_AIR_QUALITY },
    { SensorGroup::MOTION,        COMPANY_ID_MOTION },
    { SensorGroup::AMBIENT,       COMPANY_ID_AMBIENT },
    { SensorGroup::SYSTEM,        COMPANY_ID_SYSTEM },
    { SensorGroup::CURRENT,       COMPANY_ID_CURRENT }
};

// -------------------------------------------------------------
// Utility to loop over company IDs
// -------------------------------------------------------------
inline std::vector<uint16_t> getAllCompanyIds() {
    std::vector<uint16_t> ids;
    for (const auto& kv : SENSOR_COMPANY_ID_MAP) {
        ids.push_back(kv.second);
    }
    return ids;
}

inline std::string getSensorGroupName(SensorGroup group) {
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
// Manufacturer data packing helpers
// -------------------------------------------------------------
inline std::vector<uint8_t> packManufacturerData(
    const std::map<std::string, float>& sensorValues,
    const ManufacturerDataFormat& format)
{
    std::vector<uint8_t> data(2 + format.totalLength, 0);
    data[0] = static_cast<uint8_t>(format.companyId & 0xFF);
    data[1] = static_cast<uint8_t>((format.companyId >> 8) & 0xFF);

    for (const auto& field : format.dataFields) {
        auto it = sensorValues.find(field.sensorName);
        if (it == sensorValues.end()) continue;
        float val = it->second / field.scale;

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
    default: break;
}
    }

    return data;
}

inline std::vector<uint8_t> packSensorGroupData(
    const std::map<std::string, float>& sensorValues,
    SensorGroup /*group*/,
    DeviceProfile &profile)
{
    return packManufacturerData(sensorValues, profile.manufacturerFormat);
}

// -------------------------------------------------------------
// Get all device profiles
// -------------------------------------------------------------
inline std::vector<DeviceProfile> getAllProfiles() {
    return { createEnvironmentalProfile() };
}

} // namespace BLEProfiles
