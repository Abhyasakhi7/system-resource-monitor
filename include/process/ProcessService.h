#pragma once

#include "core/Metrics.h"
#include "process/ProcessInfo.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace srm {

class ProcessService {
public:
    ProcessService();

    std::vector<ProcessInfo> refresh();

    ProcessSignalResult sendSignal(int pid, int signal, std::uint64_t expectedStartTicks);

    static long clockTicksPerSecond();

private:
    std::chrono::steady_clock::time_point previousTime_;
    std::unordered_map<int, std::pair<std::uint64_t, std::chrono::steady_clock::time_point>>
        previousCpu_;
};

} // namespace srm
