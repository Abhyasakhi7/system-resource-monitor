#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace srm {

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using WallClock = std::chrono::system_clock;
using WallTimePoint = WallClock::time_point;

struct CpuMetrics {
    std::optional<double> totalUsagePercent;
    std::vector<double> perCoreUsagePercent;
    std::optional<double> frequencyMhz;
};

struct MemoryMetrics {
    std::optional<std::uint64_t> totalBytes;
    std::optional<std::uint64_t> availableBytes;
    std::optional<std::uint64_t> usedBytes;
    std::optional<std::uint64_t> cachedBytes;
    std::optional<std::uint64_t> swapTotalBytes;
    std::optional<std::uint64_t> swapUsedBytes;
};

struct FilesystemMetrics {
    std::string mountPoint;
    std::optional<std::uint64_t> totalBytes;
    std::optional<std::uint64_t> freeBytes;
};

struct DiskMetrics {
    std::vector<FilesystemMetrics> filesystems;
    std::optional<double> readBytesPerSec;
    std::optional<double> writeBytesPerSec;
};

struct NetworkInterfaceMetrics {
    std::string name;
    std::optional<std::uint64_t> rxBytes;
    std::optional<std::uint64_t> txBytes;
    std::optional<double> rxBytesPerSec;
    std::optional<double> txBytesPerSec;
};

struct NetworkMetrics {
    std::vector<NetworkInterfaceMetrics> interfaces;
    std::optional<double> totalRxBytesPerSec;
    std::optional<double> totalTxBytesPerSec;
};

struct SystemMetricsSnapshot {
    TimePoint timestamp;
    WallTimePoint wallTimestamp;
    CpuMetrics cpu;
    MemoryMetrics memory;
    DiskMetrics disk;
    NetworkMetrics network;
    bool cpuAvailable{false};
    bool memoryAvailable{false};
    bool diskAvailable{false};
    bool networkAvailable{false};
};

} // namespace srm
