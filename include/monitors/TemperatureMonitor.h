#pragma once

#include "core/Metrics.h"

#include <optional>
#include <string>

namespace srm {

std::optional<int> parseTemperatureMilliC(const std::string& text);

class TemperatureMonitor {
public:
    explicit TemperatureMonitor(std::string thermalRoot = "/sys/class/thermal",
                                std::string hwmonRoot = "/sys/class/hwmon");

    bool refresh();

    const TemperatureMetrics& metrics() const { return metrics_; }
    bool available() const { return available_; }

private:
    std::string thermalRoot_;
    std::string hwmonRoot_;
    TemperatureMetrics metrics_;
    bool available_{false};
};

} // namespace srm
