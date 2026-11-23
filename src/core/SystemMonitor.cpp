#include "core/SystemMonitor.h"

#include <QTimer>

#include <algorithm>

namespace srm {

SystemMonitor::SystemMonitor(QObject* parent)
    : QObject(parent), timer_(new QTimer(this)) {
    timer_->setInterval(intervalMs_);
    connect(timer_, &QTimer::timeout, this, &SystemMonitor::onTimeout);
}

SystemMonitor::~SystemMonitor() = default;

void SystemMonitor::start() {
    timer_->start(intervalMs_);
}

void SystemMonitor::stop() {
    timer_->stop();
    emit stopped();
}

void SystemMonitor::setInterval(int intervalMs) {
    intervalMs_ = std::max(100, intervalMs);
    if (timer_->isActive()) {
        timer_->stop();
        timer_->start(intervalMs_);
    } else {
        timer_->setInterval(intervalMs_);
    }
}

void SystemMonitor::setCollectorEnabled(bool cpu, bool memory, bool disk, bool network,
                                        bool temperature, bool battery) {
    CollectorConfig config;
    config.cpu = cpu;
    config.memory = memory;
    config.disk = disk;
    config.network = network;
    config.temperature = temperature;
    config.battery = battery;
    collector_.setConfig(config);
}

void SystemMonitor::onTimeout() {
    emit snapshotReady(collector_.collect());
}

} // namespace srm
