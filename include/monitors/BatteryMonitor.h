#pragma once

#include "core/Metrics.h"

#include <optional>
#include <string>

namespace srm {

std::optional<int> parseBatteryCapacity(const std::string& text);
std::optional<int> calculateChargePercent(const std::string& currentText,
                                          const std::string& maximumText);

class BatteryMonitor {
public:
    explicit BatteryMonitor(std::string powerSupplyRoot = "/sys/class/power_supply");

    bool refresh();

    const BatteryMetrics& metrics() const { return metrics_; }
    bool available() const { return available_; }

private:
    std::string powerSupplyRoot_;
    BatteryMetrics metrics_;
    bool available_{false};
};

} // namespace srm
