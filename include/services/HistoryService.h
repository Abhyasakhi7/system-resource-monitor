#pragma once

#include "core/Metrics.h"
#include "database/DatabaseManager.h"
#include "services/ConfigManager.h"

#include <QObject>

namespace srm {

class HistoryService : public QObject {
    Q_OBJECT

public:
    HistoryService(DatabaseManager* database, ConfigManager* config,
                   QObject* parent = nullptr);

    void recordSnapshot(const SystemMetricsSnapshot& snapshot);

    void requestSamples(const TimeRange& range);
    void requestStats(MetricKind kind, const TimeRange& range);
    void exportCsv(const TimeRange& range, const QString& path);
    void exportJson(const TimeRange& range, const QString& path);
    void setRetentionDays(int days);
    void clearHistory();

signals:
    void samplesReady(const QVector<MetricSample>& samples);
    void statsReady(MetricKind kind, const MetricStats& stats);
    void exportFinished(bool ok, const QString& path, const QString& error);
    void errorOccurred(const QString& error);

private:
    DatabaseManager* database_;
    ConfigManager* config_;
    std::int64_t lastInsertMs_{0};
};

} // namespace srm
