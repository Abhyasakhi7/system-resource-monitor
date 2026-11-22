#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace srm {

struct MetricSample {
    std::int64_t timestampMs{0};
    std::optional<double> cpuUsage;
    std::optional<std::uint64_t> memoryUsedBytes;
    std::optional<std::uint64_t> memoryTotalBytes;
    std::optional<double> diskReadRate;
    std::optional<double> diskWriteRate;
    std::optional<double> netRxRate;
    std::optional<double> netTxRate;
};

struct MetricStats {
    std::optional<double> minimum;
    std::optional<double> maximum;
    std::optional<double> average;
    std::uint64_t count{0};
};

enum class MetricKind {
    Cpu,
    Memory,
    DiskRead,
    DiskWrite,
    NetworkRx,
    NetworkTx,
};

struct TimeRange {
    std::int64_t fromMs{0};
    std::int64_t toMs{0};

    bool valid() const { return toMs >= fromMs; }
};

struct AppSetting {
    std::string key;
    std::string value;
};

} // namespace srm
