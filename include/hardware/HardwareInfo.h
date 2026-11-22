#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace srm {

enum class InfoStatus {
    Available,
    Unavailable,
    PermissionDenied,
    Unsupported,
};

struct CpuInformation {
    std::string model;
    std::string vendor;
    std::string architecture;
    int physicalCores{0};
    int logicalThreads{0};
    std::optional<double> frequencyMhz;
    bool available{false};
    std::string source;
};

struct SystemInformation {
    std::string hostname;
    std::string osPrettyName;
    std::string kernelVersion;
    std::string architecture;
    bool available{false};
    std::string source;
};

struct MemoryModuleInformation {
    std::string slot;
    std::string size;
    std::string type;
    std::string speed;
};

struct MemoryInformation {
    std::optional<std::uint64_t> totalBytes;
    std::vector<MemoryModuleInformation> modules;
    bool available{false};
    std::string source;
};

struct GpuInformation {
    std::string name;
    std::string vendorId;
    std::string deviceId;
    std::string driver;
    std::optional<std::uint64_t> vramBytes;
    bool available{false};
    std::string source;
};

struct StorageDeviceInformation {
    std::string device;
    std::string model;
    std::string vendor;
    std::string filesystem;
    std::optional<std::uint64_t> totalBytes;
    std::string mountPoint;
};

struct StorageInformation {
    std::vector<StorageDeviceInformation> devices;
    bool available{false};
    std::string source;
};

struct MotherboardInformation {
    std::string manufacturer;
    std::string model;
    std::string productName;
    bool available{false};
    std::string source;
};

struct BatteryInformation {
    std::string name;
    std::optional<int> capacityPercent;
    std::string status;
    std::string health;
    bool available{false};
    std::string source;
};

struct HardwareReport {
    CpuInformation cpu;
    SystemInformation system;
    MemoryInformation memory;
    GpuInformation gpu;
    StorageInformation storage;
    MotherboardInformation motherboard;
    BatteryInformation battery;
};

} // namespace srm
