#pragma once

#include "database/DatabaseRepository.h"
#include "database/DatabaseTypes.h"

#include <QMetaType>
#include <QObject>
#include <QString>
#include <QVector>

namespace srm {

class DatabaseManager : public QObject {
    Q_OBJECT

public:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager() override;

public slots:
    void initialize(const QString& dbPath, int retentionDays);
    void insertSample(const srm::MetricSample& sample);
    void querySamples(const srm::TimeRange& range);
    void queryStats(srm::MetricKind kind, const srm::TimeRange& range);
    void setRetentionDays(int days);
    void clearHistory();
    void exportCsv(const srm::TimeRange& range, const QString& path);
    void exportJson(const srm::TimeRange& range, const QString& path);
    void getSetting(const QString& key);
    void setSetting(const QString& key, const QString& value);
    void getAllSettings();

signals:
    void initialized(bool ok, const QString& error);
    void samplesReady(const QVector<srm::MetricSample>& samples);
    void statsReady(srm::MetricKind kind, const srm::MetricStats& stats);
    void exportFinished(bool ok, const QString& path, const QString& error);
    void settingReady(const QString& key, const QString& value);
    void allSettingsReady(const QVector<srm::AppSetting>& settings);
    void errorOccurred(const QString& error);

private:
    DatabaseRepository repository_;
};

} // namespace srm

Q_DECLARE_METATYPE(srm::MetricSample)
Q_DECLARE_METATYPE(srm::MetricStats)
Q_DECLARE_METATYPE(srm::TimeRange)
Q_DECLARE_METATYPE(srm::MetricKind)
Q_DECLARE_METATYPE(srm::AppSetting)
Q_DECLARE_METATYPE(QVector<srm::MetricSample>)
Q_DECLARE_METATYPE(QVector<srm::AppSetting>)
