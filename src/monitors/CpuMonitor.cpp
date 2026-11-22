#include "monitors/CpuMonitor.h"

#include "core/FileUtil.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace srm {

namespace {

std::uint64_t sumActiveFields(const std::vector<std::uint64_t>& values) {
    const std::size_t count = std::min<std::size_t>(values.size(), 8);
    std::uint64_t total = 0;
    for (std::size_t i = 0; i < count; ++i) {
        total += values[i];
    }
    return total;
}

bool isCoreLabel(const std::string& label) {
    if (label.size() <= 3 || label.compare(0, 3, "cpu") != 0) {
        return false;
    }
    return std::isdigit(static_cast<unsigned char>(label[3])) != 0;
}

} // namespace

CpuRawSample parseProcStat(const std::string& text) {
    CpuRawSample sample;
    std::istringstream stream(text);
    std::string line;

    while (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        std::string label;
        if (!(lineStream >> label)) {
            continue;
        }

        std::vector<std::uint64_t> values;
        std::uint64_t value = 0;
        while (lineStream >> value) {
            values.push_back(value);
        }
        if (values.size() < 4) {
            continue;
        }

        const std::uint64_t total = sumActiveFields(values);
        const std::size_t count = std::min<std::size_t>(values.size(), 8);
        const std::uint64_t idle = values[3] + (count > 4 ? values[4] : 0);

        if (label == "cpu") {
            sample.totalTicks = total;
            sample.idleTicks = idle;
            sample.valid = true;
        } else if (isCoreLabel(label)) {
            sample.cores.emplace_back(total, idle);
        }
    }

    return sample;
}

std::optional<double> computeCpuUsagePercent(const CpuRawSample& previous,
                                             const CpuRawSample& current,
                                             std::chrono::milliseconds elapsed) {
    if (elapsed.count() <= 0 || !previous.valid || !current.valid) {
        return std::nullopt;
    }
    if (current.totalTicks < previous.totalTicks || current.idleTicks < previous.idleTicks) {
        return std::nullopt;
    }

    const std::uint64_t deltaTotal = current.totalTicks - previous.totalTicks;
    const std::uint64_t deltaIdle = current.idleTicks - previous.idleTicks;
    if (deltaTotal == 0 || deltaIdle > deltaTotal) {
        return std::nullopt;
    }

    const double usage = 100.0 * static_cast<double>(deltaTotal - deltaIdle) /
                         static_cast<double>(deltaTotal);
    return std::clamp(usage, 0.0, 100.0);
}

std::vector<double> computePerCoreUsagePercent(const CpuRawSample& previous,
                                               const CpuRawSample& current) {
    if (!previous.valid || !current.valid || previous.cores.size() != current.cores.size()) {
        return {};
    }

    std::vector<double> usage;
    usage.reserve(current.cores.size());

    for (std::size_t i = 0; i < current.cores.size(); ++i) {
        const auto& previousCore = previous.cores[i];
        const auto& currentCore = current.cores[i];

        if (currentCore.first < previousCore.first || currentCore.second < previousCore.second) {
            return {};
        }
        const std::uint64_t deltaTotal = currentCore.first - previousCore.first;
        const std::uint64_t deltaIdle = currentCore.second - previousCore.second;
        if (deltaTotal == 0 || deltaIdle > deltaTotal) {
            return {};
        }

        usage.push_back(std::clamp(100.0 * static_cast<double>(deltaTotal - deltaIdle) /
                                       static_cast<double>(deltaTotal),
                                   0.0, 100.0));
    }

    return usage;
}

std::optional<double> parseFrequencyKhz(const std::string& text) {
    std::istringstream stream(text);
    double value = 0.0;
    if (!(stream >> value) || value < 0.0) {
        return std::nullopt;
    }
    return value;
}

CpuMonitor::CpuMonitor(std::string statPath, std::string frequencyPath)
    : statPath_(std::move(statPath)), frequencyPath_(std::move(frequencyPath)) {}

bool CpuMonitor::refresh() {
    const auto text = readFileText(statPath_);
    if (!text) {
        available_ = false;
        metrics_ = CpuMetrics{};
        return false;
    }

    const CpuRawSample current = parseProcStat(*text);
    if (!current.valid) {
        available_ = false;
        metrics_ = CpuMetrics{};
        return false;
    }

    const TimePoint now = Clock::now();
    if (previous_.has_value()) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - previousTime_);
        metrics_.totalUsagePercent = computeCpuUsagePercent(*previous_, current, elapsed);
        metrics_.perCoreUsagePercent = computePerCoreUsagePercent(*previous_, current);
    } else {
        metrics_.totalUsagePercent.reset();
        metrics_.perCoreUsagePercent.clear();
    }

    previous_ = current;
    previousTime_ = now;

    const auto frequencyText = readFileText(frequencyPath_);
    if (frequencyText) {
        const auto khz = parseFrequencyKhz(*frequencyText);
        metrics_.frequencyMhz = khz ? std::optional<double>(*khz / 1000.0) : std::nullopt;
    } else {
        metrics_.frequencyMhz.reset();
    }

    available_ = true;
    return true;
}

} // namespace srm
