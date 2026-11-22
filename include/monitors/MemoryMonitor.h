#pragma once

#include "core/Metrics.h"

#include <cstdint>
#include <optional>
#include <string>

namespace srm {

struct MemoryRawSample {
    std::optional<std::uint64_t> memTotalKb;
    std::optional<std::uint64_t> memAvailableKb;
    std::optional<std::uint64_t> memFreeKb;
    std::optional<std::uint64_t> buffersKb;
    std::optional<std::uint64_t> cachedKb;
    std::optional<std::uint64_t> swapTotalKb;
    std::optional<std::uint64_t> swapFreeKb;
    bool valid{false};
};

MemoryRawSample parseProcMeminfo(const std::string& text);

class MemoryMonitor {
public:
    explicit MemoryMonitor(std::string meminfoPath = "/proc/meminfo");

    bool refresh();

    const MemoryMetrics& metrics() const { return metrics_; }
    bool available() const { return available_; }

private:
    std::string meminfoPath_;
    MemoryMetrics metrics_;
    bool available_{false};
};

} // namespace srm
