#include "monitors/MemoryMonitor.h"

#include "core/FileUtil.h"

#include <sstream>
#include <utility>

namespace srm {

MemoryRawSample parseProcMeminfo(const std::string& text) {
    MemoryRawSample sample;
    std::istringstream stream(text);
    std::string line;

    while (std::getline(stream, line)) {
        const std::string::size_type colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }

        const std::string key = line.substr(0, colon);
        std::istringstream values(line.substr(colon + 1));
        std::uint64_t value = 0;
        if (!(values >> value)) {
            continue;
        }

        if (key == "MemTotal") {
            sample.memTotalKb = value;
        } else if (key == "MemAvailable") {
            sample.memAvailableKb = value;
        } else if (key == "MemFree") {
            sample.memFreeKb = value;
        } else if (key == "Buffers") {
            sample.buffersKb = value;
        } else if (key == "Cached") {
            sample.cachedKb = value;
        } else if (key == "SwapTotal") {
            sample.swapTotalKb = value;
        } else if (key == "SwapFree") {
            sample.swapFreeKb = value;
        }
    }

    sample.valid = sample.memTotalKb.has_value();
    return sample;
}

MemoryMonitor::MemoryMonitor(std::string meminfoPath)
    : meminfoPath_(std::move(meminfoPath)) {}

bool MemoryMonitor::refresh() {
    const auto text = readFileText(meminfoPath_);
    if (!text) {
        available_ = false;
        metrics_ = MemoryMetrics{};
        return false;
    }

    const MemoryRawSample raw = parseProcMeminfo(*text);
    if (!raw.valid) {
        available_ = false;
        metrics_ = MemoryMetrics{};
        return false;
    }

    metrics_ = MemoryMetrics{};

    const auto toBytes = [](const std::optional<std::uint64_t>& kb)
        -> std::optional<std::uint64_t> {
        if (!kb) {
            return std::nullopt;
        }
        return *kb * 1024ULL;
    };

    metrics_.totalBytes = toBytes(raw.memTotalKb);
    metrics_.availableBytes = toBytes(raw.memAvailableKb);

    if (metrics_.totalBytes && metrics_.availableBytes &&
        *metrics_.availableBytes <= *metrics_.totalBytes) {
        metrics_.usedBytes = *metrics_.totalBytes - *metrics_.availableBytes;
    }

    if (raw.cachedKb && raw.buffersKb) {
        metrics_.cachedBytes = (*raw.cachedKb + *raw.buffersKb) * 1024ULL;
    } else {
        metrics_.cachedBytes = toBytes(raw.cachedKb);
    }

    metrics_.swapTotalBytes = toBytes(raw.swapTotalKb);
    if (raw.swapTotalKb && raw.swapFreeKb && *raw.swapFreeKb <= *raw.swapTotalKb) {
        metrics_.swapUsedBytes = (*raw.swapTotalKb - *raw.swapFreeKb) * 1024ULL;
    }

    available_ = true;
    return true;
}

} // namespace srm
