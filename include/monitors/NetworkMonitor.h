#pragma once

#include "core/Metrics.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace srm {

struct NetworkRawInterface {
    std::string name;
    std::uint64_t rxBytes{0};
    std::uint64_t txBytes{0};
};

struct NetworkRawSample {
    std::vector<NetworkRawInterface> interfaces;
    bool valid{false};
};

NetworkRawSample parseProcNetDev(const std::string& text);

class NetworkMonitor {
public:
    explicit NetworkMonitor(std::string netDevPath = "/proc/net/dev");

    bool refresh();

    const NetworkMetrics& metrics() const { return metrics_; }
    bool available() const { return available_; }

private:
    std::string netDevPath_;
    std::optional<NetworkRawSample> previous_;
    TimePoint previousTime_;
    NetworkMetrics metrics_;
    bool available_{false};
};

} // namespace srm
