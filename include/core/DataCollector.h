#pragma once

#include "core/Metrics.h"
#include "monitors/CpuMonitor.h"
#include "monitors/DiskMonitor.h"
#include "monitors/MemoryMonitor.h"
#include "monitors/NetworkMonitor.h"
#include "monitors/BatteryMonitor.h"
#include "monitors/TemperatureMonitor.h"

#include <string>

namespace srm {

struct CollectorPaths {
    std::string statPath{"/proc/stat"};
    std::string meminfoPath{"/proc/meminfo"};
    std::string diskstatsPath{"/proc/diskstats"};
    std::string netDevPath{"/proc/net/dev"};
    std::string thermalRoot{"/sys/class/thermal"};
    std::string hwmonRoot{"/sys/class/hwmon"};
    std::string powerSupplyRoot{"/sys/class/power_supply"};
};

struct CollectorConfig {
    bool cpu{true};
    bool memory{true};
    bool disk{true};
    bool network{true};
    bool temperature{true};
    bool battery{true};
};

class DataCollector {
public:
    explicit DataCollector(const CollectorPaths& paths = CollectorPaths{},
                           const CollectorConfig& config = CollectorConfig{});

    void setConfig(const CollectorConfig& config) { config_ = config; }
    const CollectorConfig& config() const { return config_; }

    SystemMetricsSnapshot collect();

private:
    CpuMonitor cpu_;
    MemoryMonitor memory_;
    DiskMonitor disk_;
    NetworkMonitor network_;
    TemperatureMonitor temperature_;
    BatteryMonitor battery_;
    CollectorConfig config_;
};

} // namespace srm
