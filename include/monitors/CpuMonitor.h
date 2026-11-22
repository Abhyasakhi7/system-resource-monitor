#pragma once

#include "core/Metrics.h"

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace srm {

struct CpuRawSample {
    std::uint64_t totalTicks{0};
    std::uint64_t idleTicks{0};
    std::vector<std::pair<std::uint64_t, std::uint64_t>> cores;
    bool valid{false};
};

CpuRawSample parseProcStat(const std::string& text);

std::optional<double> computeCpuUsagePercent(const CpuRawSample& previous,
                                             const CpuRawSample& current,
                                             std::chrono::milliseconds elapsed);

std::vector<double> computePerCoreUsagePercent(const CpuRawSample& previous,
                                               const CpuRawSample& current);

std::optional<double> parseFrequencyKhz(const std::string& text);

class CpuMonitor {
public:
    explicit CpuMonitor(std::string statPath = "/proc/stat",
                        std::string frequencyPath = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");

    bool refresh();

    const CpuMetrics& metrics() const { return metrics_; }
    bool available() const { return available_; }

private:
    std::string statPath_;
    std::string frequencyPath_;
    std::optional<CpuRawSample> previous_;
    TimePoint previousTime_;
    CpuMetrics metrics_;
    bool available_{false};
};

} // namespace srm
