#include "process/ProcessService.h"

#include "core/FileUtil.h"

#include <QDir>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <pwd.h>
#include <sstream>
#include <unistd.h>

namespace srm {

namespace {

std::string readValue(const std::string& text, const std::string& key) {
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        const auto colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        std::string name = line.substr(0, colon);
        const auto first = name.find_first_not_of(" \t");
        const auto last = name.find_last_not_of(" \t");
        if (first == std::string::npos) {
            continue;
        }
        name = name.substr(first, last - first + 1);
        if (name == key) {
            std::string value = line.substr(colon + 1);
            const auto valueFirst = value.find_first_not_of(" \t");
            const auto valueLast = value.find_last_not_of(" \r\n \t");
            if (valueFirst == std::string::npos) {
                return {};
            }
            return value.substr(valueFirst, valueLast - valueFirst + 1);
        }
    }
    return {};
}

std::string stateText(const std::string& state) {
    if (state.empty()) {
        return {};
    }
    switch (state[0]) {
        case 'R':
            return "Running";
        case 'S':
            return "Sleeping";
        case 'D':
            return "Disk wait";
        case 'Z':
            return "Zombie";
        case 'T':
            return "Stopped";
        case 't':
            return "Tracing stop";
        case 'X':
        case 'x':
            return "Dead";
        case 'I':
            return "Idle";
        default:
            return "Unknown";
    }
}

std::vector<int> listPids() {
    std::vector<int> pids;
    QDir proc(QStringLiteral("/proc"));
    const QStringList entries = proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& entry : entries) {
        bool ok = false;
        const int pid = entry.toInt(&ok);
        if (ok && pid > 0) {
            pids.push_back(pid);
        }
    }
    std::sort(pids.begin(), pids.end());
    return pids;
}

} // namespace

ProcessService::ProcessService()
    : previousTime_(std::chrono::steady_clock::now()) {}

long ProcessService::clockTicksPerSecond() {
    const long ticks = sysconf(_SC_CLK_TCK);
    return ticks > 0 ? ticks : 100;
}

std::vector<ProcessInfo> ProcessService::refresh() {
    std::vector<ProcessInfo> processes;
    const auto now = std::chrono::steady_clock::now();
    const double elapsedSeconds =
        std::chrono::duration_cast<std::chrono::duration<double>>(now - previousTime_).count();
    const double ticksPerSecond = static_cast<double>(clockTicksPerSecond());

    std::unordered_map<int, bool> seen;

    for (const int pid : listPids()) {
        seen[pid] = true;
        const std::string statPath = "/proc/" + std::to_string(pid) + "/stat";
        const auto statText = readFileText(statPath);
        if (!statText) {
            continue;
        }

        const ProcessStat stat = parseProcessStat(*statText);
        if (!stat.valid) {
            continue;
        }

        ProcessInfo info;
        info.pid = stat.pid;
        info.name = stat.name;
        info.state = stateText(stat.state);
        info.parentPid = stat.parentPid;
        info.threadCount = stat.threadCount;
        info.startTicks = stat.startTicks;
        info.statusText = stat.state;

        const std::string statusPath = "/proc/" + std::to_string(pid) + "/status";
        if (const auto statusText = readFileText(statusPath)) {
            const std::string uid = readValue(*statusText, "Uid");
            if (!uid.empty()) {
                std::istringstream uidStream(uid);
                unsigned int uidValue = 0;
                if (uidStream >> uidValue) {
                    if (const passwd* entry = getpwuid(uidValue)) {
                        info.user = entry->pw_name;
                    }
                }
            }

            const std::string vmRss = readValue(*statusText, "VmRSS");
            if (!vmRss.empty()) {
                std::istringstream stream(vmRss);
                std::uint64_t kb = 0;
                if (stream >> kb) {
                    info.rssBytes = kb * 1024ULL;
                }
            }

            const std::string vmSize = readValue(*statusText, "VmSize");
            if (!vmSize.empty()) {
                std::istringstream stream(vmSize);
                std::uint64_t kb = 0;
                if (stream >> kb) {
                    info.vmSizeBytes = kb * 1024ULL;
                }
            }

            const std::string name = readValue(*statusText, "Name");
            if (!name.empty()) {
                info.name = name;
            }
        }

        const std::uint64_t cpuTicks = stat.utime + stat.stime;
        const auto previous = previousCpu_.find(pid);
        if (previous != previousCpu_.end() && elapsedSeconds > 0.0) {
            const std::uint64_t previousTicks = previous->second.first;
            if (cpuTicks >= previousTicks) {
                const double cpuSeconds =
                    static_cast<double>(cpuTicks - previousTicks) / ticksPerSecond;
                info.cpuPercent = (cpuSeconds / elapsedSeconds) * 100.0;
            }
        }
        previousCpu_[pid] = {cpuTicks, now};

        processes.push_back(std::move(info));
    }

    for (auto it = previousCpu_.begin(); it != previousCpu_.end();) {
        if (seen.find(it->first) == seen.end()) {
            it = previousCpu_.erase(it);
        } else {
            ++it;
        }
    }

    previousTime_ = now;
    return processes;
}

ProcessSignalResult ProcessService::sendSignal(int pid, int signal,
                                               std::uint64_t expectedStartTicks) {
    if (pid <= 0) {
        return ProcessSignalResult::InvalidPid;
    }
    if (signal != SIGTERM && signal != SIGKILL) {
        return ProcessSignalResult::Failed;
    }

    const auto statText = readFileText("/proc/" + std::to_string(pid) + "/stat");
    if (!statText) {
        return ProcessSignalResult::NoSuchProcess;
    }

    const ProcessStat stat = parseProcessStat(*statText);
    if (!stat.valid || stat.pid != pid) {
        return ProcessSignalResult::NoSuchProcess;
    }
    if (expectedStartTicks != 0 && stat.startTicks != expectedStartTicks) {
        return ProcessSignalResult::NoSuchProcess;
    }

    if (::kill(pid, signal) != 0) {
        switch (errno) {
            case ESRCH:
                return ProcessSignalResult::NoSuchProcess;
            case EPERM:
                return ProcessSignalResult::PermissionDenied;
            default:
                return ProcessSignalResult::Failed;
        }
    }
    return ProcessSignalResult::Ok;
}

} // namespace srm
