#include "core/DataCollector.h"

#include <utility>

namespace srm {

DataCollector::DataCollector(const CollectorPaths& paths, const CollectorConfig& config)
    : cpu_(paths.statPath), memory_(paths.meminfoPath), disk_(paths.diskstatsPath),
      network_(paths.netDevPath), temperature_(paths.thermalRoot, paths.hwmonRoot),
      battery_(paths.powerSupplyRoot), config_(config) {}

SystemMetricsSnapshot DataCollector::collect() {
    SystemMetricsSnapshot snapshot;
    snapshot.timestamp = Clock::now();
    snapshot.wallTimestamp = WallClock::now();

    if (config_.cpu) {
        snapshot.cpuAvailable = cpu_.refresh();
        snapshot.cpu = cpu_.metrics();
    }

    if (config_.memory) {
        snapshot.memoryAvailable = memory_.refresh();
        snapshot.memory = memory_.metrics();
    }

    if (config_.disk) {
        snapshot.diskAvailable = disk_.refresh();
        snapshot.disk = disk_.metrics();
    }

    if (config_.network) {
        snapshot.networkAvailable = network_.refresh();
        snapshot.network = network_.metrics();
    }

    if (config_.temperature) {
        snapshot.temperatureAvailable = temperature_.refresh();
        snapshot.temperature = temperature_.metrics();
    }

    if (config_.battery) {
        snapshot.batteryAvailable = battery_.refresh();
        snapshot.battery = battery_.metrics();
    }

    return snapshot;
}

} // namespace srm
