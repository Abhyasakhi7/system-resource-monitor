#include "database/DatabaseManager.h"

#include <QVector>

namespace srm {

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent) {}

DatabaseManager::~DatabaseManager() = default;

void DatabaseManager::initialize(const QString& dbPath, int retentionDays) {
    repository_.setRetentionDays(retentionDays);
    if (!repository_.open(dbPath.toStdString())) {
        emit initialized(false, QString::fromStdString(repository_.lastError()));
        return;
    }
    if (!repository_.initializeSchema()) {
        emit initialized(false, QString::fromStdString(repository_.lastError()));
        return;
    }
    if (!repository_.prune()) {
        emit errorOccurred(QString::fromStdString(repository_.lastError()));
    }
    emit initialized(true, QString());
}

void DatabaseManager::insertSample(const MetricSample& sample) {
    if (!repository_.insertSample(sample)) {
        emit errorOccurred(QString::fromStdString(repository_.lastError()));
    }
}

void DatabaseManager::querySamples(const TimeRange& range) {
    const std::vector<MetricSample> samples = repository_.querySamples(range);
    QVector<MetricSample> result;
    result.reserve(static_cast<int>(samples.size()));
    for (const MetricSample& sample : samples) {
        result.append(sample);
    }
    emit samplesReady(result);
}

void DatabaseManager::queryStats(MetricKind kind, const TimeRange& range) {
    emit statsReady(kind, repository_.queryStats(kind, range));
}

void DatabaseManager::setRetentionDays(int days) {
    repository_.setRetentionDays(days);
    if (!repository_.prune()) {
        emit errorOccurred(QString::fromStdString(repository_.lastError()));
    }
}

void DatabaseManager::clearHistory() {
    if (!repository_.clearHistory()) {
        emit errorOccurred(QString::fromStdString(repository_.lastError()));
    }
}

void DatabaseManager::exportCsv(const TimeRange& range, const QString& path) {
    std::string error;
    if (!repository_.exportCsv(range, path.toStdString(), error)) {
        emit exportFinished(false, path, QString::fromStdString(error));
        return;
    }
    emit exportFinished(true, path, QString());
}

void DatabaseManager::exportJson(const TimeRange& range, const QString& path) {
    std::string error;
    if (!repository_.exportJson(range, path.toStdString(), error)) {
        emit exportFinished(false, path, QString::fromStdString(error));
        return;
    }
    emit exportFinished(true, path, QString());
}

void DatabaseManager::getSetting(const QString& key) {
    const auto value = repository_.getSetting(key.toStdString());
    emit settingReady(key, value ? QString::fromStdString(*value) : QString());
}

void DatabaseManager::setSetting(const QString& key, const QString& value) {
    if (!repository_.setSetting(key.toStdString(), value.toStdString())) {
        emit errorOccurred(QString::fromStdString(repository_.lastError()));
    }
}

void DatabaseManager::getAllSettings() {
    const std::vector<AppSetting> settings = repository_.allSettings();
    QVector<AppSetting> result;
    result.reserve(static_cast<int>(settings.size()));
    for (const AppSetting& setting : settings) {
        result.append(setting);
    }
    emit allSettingsReady(result);
}

} // namespace srm
