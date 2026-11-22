#include "monitors/NetworkMonitor.h"

#include "core/FileUtil.h"
#include "core/MetricMath.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <utility>

namespace srm {

namespace {

std::string trim(const std::string& text) {
    const auto first = std::find_if_not(text.begin(), text.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    });
    const auto last = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char c) {
        return std::isspace(c) != 0;
    }).base();
    if (first >= last) {
        return {};
    }
    return std::string(first, last);
}

const NetworkRawInterface* findInterface(const NetworkRawSample& sample, const std::string& name) {
    const auto it = std::find_if(sample.interfaces.begin(), sample.interfaces.end(),
                                 [&name](const NetworkRawInterface& iface) {
                                     return iface.name == name;
                                 });
    return it == sample.interfaces.end() ? nullptr : &*it;
}

} // namespace

NetworkRawSample parseProcNetDev(const std::string& text) {
    NetworkRawSample sample;
    std::istringstream stream(text);
    std::string line;

    while (std::getline(stream, line)) {
        const std::string::size_type colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }

        const std::string name = trim(line.substr(0, colon));
        if (name.empty()) {
            continue;
        }

        std::istringstream values(line.substr(colon + 1));
        std::uint64_t rxBytes = 0;
        std::uint64_t rxPackets = 0;
        std::uint64_t rxErrors = 0;
        std::uint64_t rxDropped = 0;
        std::uint64_t rxFifo = 0;
        std::uint64_t rxFrame = 0;
        std::uint64_t rxCompressed = 0;
        std::uint64_t rxMulticast = 0;
        std::uint64_t txBytes = 0;
        std::uint64_t txPackets = 0;
        std::uint64_t txErrors = 0;
        std::uint64_t txDropped = 0;
        std::uint64_t txFifo = 0;
        std::uint64_t txCollisions = 0;
        std::uint64_t txCarrier = 0;
        std::uint64_t txCompressed = 0;

        if (!(values >> rxBytes >> rxPackets >> rxErrors >> rxDropped >> rxFifo >> rxFrame >>
              rxCompressed >> rxMulticast >> txBytes >> txPackets >> txErrors >> txDropped >>
              txFifo >> txCollisions >> txCarrier >> txCompressed)) {
            continue;
        }

        sample.interfaces.push_back({name, rxBytes, txBytes});
        sample.valid = true;
    }

    return sample;
}

NetworkMonitor::NetworkMonitor(std::string netDevPath)
    : netDevPath_(std::move(netDevPath)) {}

bool NetworkMonitor::refresh() {
    const auto text = readFileText(netDevPath_);
    if (!text) {
        available_ = false;
        metrics_ = NetworkMetrics{};
        return false;
    }

    const NetworkRawSample current = parseProcNetDev(*text);
    if (!current.valid) {
        available_ = false;
        metrics_ = NetworkMetrics{};
        return false;
    }

    const TimePoint now = Clock::now();
    metrics_.interfaces.clear();
    std::optional<double> totalRx;
    std::optional<double> totalTx;

    for (const NetworkRawInterface& iface : current.interfaces) {
        NetworkInterfaceMetrics entry;
        entry.name = iface.name;
        entry.rxBytes = iface.rxBytes;
        entry.txBytes = iface.txBytes;

        if (previous_.has_value()) {
            const NetworkRawInterface* previous = findInterface(*previous_, iface.name);
            if (previous != nullptr) {
                const auto elapsed =
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - previousTime_);
                entry.rxBytesPerSec = computeByteRate(previous->rxBytes, iface.rxBytes, elapsed);
                entry.txBytesPerSec = computeByteRate(previous->txBytes, iface.txBytes, elapsed);
            }
        }

        if (iface.name != "lo") {
            if (entry.rxBytesPerSec) {
                totalRx = totalRx.value_or(0.0) + *entry.rxBytesPerSec;
            }
            if (entry.txBytesPerSec) {
                totalTx = totalTx.value_or(0.0) + *entry.txBytesPerSec;
            }
        }

        metrics_.interfaces.push_back(std::move(entry));
    }

    metrics_.totalRxBytesPerSec = totalRx;
    metrics_.totalTxBytesPerSec = totalTx;
    previous_ = current;
    previousTime_ = now;
    available_ = true;
    return true;
}

} // namespace srm
