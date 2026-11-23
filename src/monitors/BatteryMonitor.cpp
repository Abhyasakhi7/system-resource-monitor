#include "monitors/BatteryMonitor.h"

#include "core/FileUtil.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <utility>
#include <vector>

namespace srm {

namespace {

std::vector<std::string> listDirectories(const std::string& root) {
    std::vector<std::string> names;
    std::error_code error;
    std::filesystem::directory_iterator iterator(root, error);
    const std::filesystem::directory_iterator end;
    for (; !error && iterator != end; iterator.increment(error)) {
        if (iterator->is_directory(error)) {
            names.push_back(iterator->path().filename().string());
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::optional<double> parsePositiveNumber(const std::string& text) {
    const std::string value = trimText(text);
    if (value.empty()) {
        return std::nullopt;
    }

    double number = 0.0;
    const char* begin = value.data();
    const char* end = begin + value.size();
    const auto result = std::from_chars(begin, end, number);
    if (result.ec != std::errc{} || result.ptr != end || !std::isfinite(number) || number < 0.0) {
        return std::nullopt;
    }
    return number;
}

} // namespace

std::optional<int> parseBatteryCapacity(const std::string& text) {
    const auto value = parsePositiveNumber(text);
    if (!value || *value > 100.0) {
        return std::nullopt;
    }
    return static_cast<int>(std::lround(*value));
}

std::optional<int> calculateChargePercent(const std::string& currentText,
                                          const std::string& maximumText) {
    const auto current = parsePositiveNumber(currentText);
    const auto maximum = parsePositiveNumber(maximumText);
    if (!current || !maximum || *maximum <= 0.0 || *current > *maximum * 2.0) {
        return std::nullopt;
    }
    const double percent = 100.0 * *current / *maximum;
    return static_cast<int>(std::lround(std::clamp(percent, 0.0, 100.0)));
}

BatteryMonitor::BatteryMonitor(std::string powerSupplyRoot)
    : powerSupplyRoot_(std::move(powerSupplyRoot)) {}

bool BatteryMonitor::refresh() {
    for (const std::string& name : listDirectories(powerSupplyRoot_)) {
        if (name.rfind("BAT", 0) != 0) {
            continue;
        }

        const std::string base = powerSupplyRoot_ + "/" + name;
        BatteryMetrics battery;
        battery.name = readTrimmedFileText(base + "/name").value_or(name);
        if (battery.name.empty()) {
            battery.name = name;
        }
        battery.status = readTrimmedFileText(base + "/status").value_or("Unknown");
        if (battery.status.empty()) {
            battery.status = "Unknown";
        }
        battery.health = readTrimmedFileText(base + "/health").value_or("");

        const auto capacityText = readTrimmedFileText(base + "/capacity");
        battery.capacityPercent = capacityText ? parseBatteryCapacity(*capacityText) : std::nullopt;
        if (!battery.capacityPercent) {
            const auto energyNow = readTrimmedFileText(base + "/energy_now");
            const auto energyFull = readTrimmedFileText(base + "/energy_full");
            if (energyNow && energyFull) {
                battery.capacityPercent = calculateChargePercent(*energyNow, *energyFull);
            }
        }
        if (!battery.capacityPercent) {
            const auto chargeNow = readTrimmedFileText(base + "/charge_now");
            const auto chargeFull = readTrimmedFileText(base + "/charge_full");
            if (chargeNow && chargeFull) {
                battery.capacityPercent = calculateChargePercent(*chargeNow, *chargeFull);
            }
        }

        available_ = battery.capacityPercent.has_value() || battery.status != "Unknown";
        metrics_ = std::move(battery);
        return available_;
    }

    available_ = false;
    metrics_ = BatteryMetrics{};
    return false;
}

} // namespace srm
