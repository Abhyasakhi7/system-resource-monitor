#pragma once

#include "core/Metrics.h"

#include <cstdint>
#include <optional>
#include <string>

namespace srm {

struct DiskRawSample {
    std::uint64_t sectorsRead{0};
    std::uint64_t sectorsWritten{0};
    bool valid{false};
};

DiskRawSample parseProcDiskstats(const std::string& text);

class DiskMonitor {
public:
    explicit DiskMonitor(std::string diskstatsPath = "/proc/diskstats");

    bool refresh();

    const DiskMetrics& metrics() const { return metrics_; }
    bool available() const { return available_; }

private:
    void refreshFilesystems();

    std::string diskstatsPath_;
    std::optional<DiskRawSample> previous_;
    TimePoint previousTime_;
    DiskMetrics metrics_;
    bool available_{false};
};

} // namespace srm
