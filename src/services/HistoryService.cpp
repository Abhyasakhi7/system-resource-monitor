#include "services/HistoryService.h"

#include <QMetaObject>
#include <QVector>

#include <chrono>

namespace srm {

HistoryService::HistoryService(DatabaseManager* database, ConfigManager* config,
                               QObject* parent)
    : QObject(parent), database_(database), config_(config) {
    connect(database_, &DatabaseManager::samplesReady, this, &HistoryService::samplesReady);
    connect(database_, &DatabaseManager::statsReady, this, &HistoryService::statsReady);
    connect(database_, &DatabaseManager::exportFinished, this,
            &HistoryService::exportFinished);
    connect(database_, &DatabaseManager::errorOccurred, this, &HistoryService::errorOccurred);
}

void HistoryService::recordSnapshot(const SystemMetricsSnapshot& snapshot) {
    if (config_ == nullptr || !config_->historyEnabled()) {
        return;
    }

    const std::int64_t timestampMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            snapshot.wallTimestamp.time_since_epoch())
            .count();

    const std::int64_t intervalMs = config_->historySampleIntervalMs();
    if (lastInsertMs_ != 0 && timestampMs - lastInsertMs_ < intervalMs) {
        return;
    }

    MetricSample sample;
    sample.timestampMs = timestampMs;
    if (snapshot.cpuAvailable) {
        sample.cpuUsage = snapshot.cpu.totalUsagePercent;
    }
    if (snapshot.memoryAvailable) {
        sample.memoryUsedBytes = snapshot.memory.usedBytes;
        sample.memoryTotalBytes = snapshot.memory.totalBytes;
    }
    if (snapshot.diskAvailable) {
        sample.diskReadRate = snapshot.disk.readBytesPerSec;
        sample.diskWriteRate = snapshot.disk.writeBytesPerSec;
    }
    if (snapshot.networkAvailable) {
        sample.netRxRate = snapshot.network.totalRxBytesPerSec;
        sample.netTxRate = snapshot.network.totalTxBytesPerSec;
    }

    lastInsertMs_ = timestampMs;
    if (database_ != nullptr) {
        QMetaObject::invokeMethod(database_, "insertSample", Qt::QueuedConnection,
                                  Q_ARG(srm::MetricSample, sample));
    }
}

void HistoryService::requestSamples(const TimeRange& range) {
    QMetaObject::invokeMethod(database_, "querySamples", Qt::QueuedConnection,
                              Q_ARG(srm::TimeRange, range));
}

void HistoryService::requestStats(MetricKind kind, const TimeRange& range) {
    QMetaObject::invokeMethod(database_, "queryStats", Qt::QueuedConnection,
                              Q_ARG(srm::MetricKind, kind),
                              Q_ARG(srm::TimeRange, range));
}

void HistoryService::exportCsv(const TimeRange& range, const QString& path) {
    QMetaObject::invokeMethod(database_, "exportCsv", Qt::QueuedConnection,
                              Q_ARG(srm::TimeRange, range), Q_ARG(QString, path));
}

void HistoryService::exportJson(const TimeRange& range, const QString& path) {
    QMetaObject::invokeMethod(database_, "exportJson", Qt::QueuedConnection,
                              Q_ARG(srm::TimeRange, range), Q_ARG(QString, path));
}

void HistoryService::setRetentionDays(int days) {
    QMetaObject::invokeMethod(database_, "setRetentionDays", Qt::QueuedConnection,
                              Q_ARG(int, days));
}

void HistoryService::clearHistory() {
    QMetaObject::invokeMethod(database_, "clearHistory", Qt::QueuedConnection);
}

} // namespace srm
