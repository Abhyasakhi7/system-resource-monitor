#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace srm {

struct ProcessInfo {
    int pid{0};
    std::string name;
    std::string user;
    std::string state;
    std::optional<double> cpuPercent;
    std::optional<std::uint64_t> rssBytes;
    std::optional<std::uint64_t> vmSizeBytes;
    std::uint64_t startTicks{0};
    int parentPid{0};
    int threadCount{0};
    std::string statusText;
    bool accessible{true};
};

enum class ProcessSignalResult {
    Ok,
    NoSuchProcess,
    PermissionDenied,
    InvalidPid,
    Failed,
};

struct ProcessStat {
    int pid{0};
    std::string name;
    std::string state;
    int parentPid{0};
    std::uint64_t utime{0};
    std::uint64_t stime{0};
    std::uint64_t startTicks{0};
    int threadCount{0};
    bool valid{false};
};

ProcessStat parseProcessStat(const std::string& text);

} // namespace srm
