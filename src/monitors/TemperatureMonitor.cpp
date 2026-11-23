#include "monitors/TemperatureMonitor.h"

#include "core/FileUtil.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <filesystem>
#include <utility>
#include <vector>

namespace srm {

namespace {

constexpr std::size_t kMaxSensors = 32;

std::vector<std::string> listEntries(const std::string& root) {
    std::vector<std::string> names;
    std::error_code error;
    std::filesystem::directory_iterator iterator(root, error);
    const std::filesystem::directory_iterator end;
    for (; !error && iterator != end; iterator.increment(error)) {
        names.push_back(iterator->path().filename().string());
    }
    std::sort(names.begin(), names.end());
    return names;
}

bool isThermalZone(const std::string& name) {
    return name.size() > 12 && name.compare(0, 12, "thermal_zone") == 0 &&
           std::all_of(name.begin() + 12, name.end(), [](unsigned char character) {
               return std::isdigit(character) != 0;
           });
}

bool isHwmonDevice(const std::string& name) {
    return name.size() > 5 && name.compare(0, 5, "hwmon") == 0 &&
           std::all_of(name.begin() + 5, name.end(), [](unsigned char character) {
               return std::isdigit(character) != 0;
           });
}

std::optional<std::string> temperatureInputIndex(const std::string& name) {
    if (name.size() <= 10 || name.compare(0, 4, "temp") != 0 ||
        name.compare(name.size() - 6, 6, "_input") != 0) {
        return std::nullopt;
    }
    const std::string index = name.substr(4, name.size() - 10);
    if (!std::all_of(index.begin(), index.end(), [](unsigned char character) {
            return std::isdigit(character) != 0;
        })) {
        return std::nullopt;
    }
    return index;
}

} // namespace

std::optional<int> parseTemperatureMilliC(const std::string& text) {
    const std::string value = trimText(text);
    if (value.empty()) {
        return std::nullopt;
    }

    int millidegrees = 0;
    const char* begin = value.data();
    const char* end = begin + value.size();
    const auto result = std::from_chars(begin, end, millidegrees);
    if (result.ec != std::errc{} || result.ptr != end) {
        return std::nullopt;
    }
    if (millidegrees < -100000 || millidegrees > 200000) {
        return std::nullopt;
    }
    return millidegrees;
}

TemperatureMonitor::TemperatureMonitor(std::string thermalRoot, std::string hwmonRoot)
    : thermalRoot_(std::move(thermalRoot)), hwmonRoot_(std::move(hwmonRoot)) {}

bool TemperatureMonitor::refresh() {
    TemperatureMetrics current;

    for (const std::string& zone : listEntries(thermalRoot_)) {
        if (!isThermalZone(zone)) {
            continue;
        }
        const std::string base = thermalRoot_ + "/" + zone;
        const auto raw = readTrimmedFileText(base + "/temp");
        if (!raw) {
            continue;
        }
        const auto millidegrees = parseTemperatureMilliC(*raw);
        if (!millidegrees) {
            continue;
        }
        const std::string type = readTrimmedFileText(base + "/type").value_or(zone);
        current.sensors.push_back(
            {type.empty() ? zone : type, static_cast<double>(*millidegrees) / 1000.0});
    }

    for (const std::string& device : listEntries(hwmonRoot_)) {
        if (!isHwmonDevice(device)) {
            continue;
        }
        const std::string base = hwmonRoot_ + "/" + device;
        const std::string driver = readTrimmedFileText(base + "/name").value_or(device);
        for (const std::string& entry : listEntries(base)) {
            const auto index = temperatureInputIndex(entry);
            if (!index) {
                continue;
            }
            const auto raw = readTrimmedFileText(base + "/" + entry);
            if (!raw) {
                continue;
            }
            const auto millidegrees = parseTemperatureMilliC(*raw);
            if (!millidegrees) {
                continue;
            }
            const std::string label =
                readTrimmedFileText(base + "/temp" + *index + "_label").value_or("temp" + *index);
            current.sensors.push_back({driver + " " + label,
                                       static_cast<double>(*millidegrees) / 1000.0});
        }
    }

    std::sort(current.sensors.begin(), current.sensors.end(),
              [](const TemperatureSensorMetrics& left, const TemperatureSensorMetrics& right) {
                  return left.name < right.name;
              });
    if (current.sensors.size() > kMaxSensors) {
        current.sensors.resize(kMaxSensors);
    }

    available_ = !current.sensors.empty();
    metrics_ = std::move(current);
    return available_;
}

} // namespace srm
