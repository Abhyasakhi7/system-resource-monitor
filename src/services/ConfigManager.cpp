#include "services/ConfigManager.h"

#include <QMetaObject>
#include <QStringList>
#include <QVector>

#include <algorithm>

namespace srm {

namespace {

constexpr int kDefaultIntervalMs = 1000;
constexpr int kDefaultHistorySampleIntervalMs = 1000;
constexpr int kDefaultRetentionDays = 7;

QString defaultValue(const std::string& key) {
    if (key == "interval_ms") {
        return QString::number(kDefaultIntervalMs);
    }
    if (key == "history_enabled") {
        return QStringLiteral("true");
    }
    if (key == "history_sample_interval_ms") {
        return QString::number(kDefaultHistorySampleIntervalMs);
    }
    if (key == "retention_days") {
        return QString::number(kDefaultRetentionDays);
    }
    if (key == "theme") {
        return QStringLiteral("dark");
    }
    if (key == "network_unit") {
        return QStringLiteral("bytes");
    }
    if (key == "collector_cpu" || key == "collector_memory" || key == "collector_disk" ||
        key == "collector_network") {
        return QStringLiteral("true");
    }
    return QString();
}

} // namespace

ConfigManager::ConfigManager(DatabaseManager* database, QObject* parent)
    : QObject(parent), database_(database) {
    connect(database_, &DatabaseManager::allSettingsReady, this,
            &ConfigManager::onAllSettings);
    connect(database_, &DatabaseManager::settingReady, this,
            &ConfigManager::onSettingReady);
}

void ConfigManager::load() {
    QMetaObject::invokeMethod(database_, "getAllSettings", Qt::QueuedConnection);
}

std::string ConfigManager::setting(const char* key) const {
    const auto it = values_.find(key);
    if (it != values_.end()) {
        return it->second;
    }
    const QString fallback = defaultValue(key);
    return fallback.toStdString();
}

int ConfigManager::intSetting(const char* key, int fallback, int minimum, int maximum) const {
    const std::string raw = setting(key);
    bool ok = false;
    const int value = QString::fromStdString(raw).toInt(&ok);
    if (!ok) {
        return fallback;
    }
    return std::clamp(value, minimum, maximum);
}

bool ConfigManager::boolSetting(const char* key, bool fallback) const {
    const std::string raw = setting(key);
    const QString value = QString::fromStdString(raw).trimmed().toLower();
    if (value == QLatin1String("true") || value == QLatin1String("1") ||
        value == QLatin1String("yes")) {
        return true;
    }
    if (value == QLatin1String("false") || value == QLatin1String("0") ||
        value == QLatin1String("no")) {
        return false;
    }
    return fallback;
}

int ConfigManager::intervalMs() const {
    return intSetting("interval_ms", kDefaultIntervalMs, 100, 10000);
}

bool ConfigManager::historyEnabled() const {
    return boolSetting("history_enabled", true);
}

int ConfigManager::historySampleIntervalMs() const {
    return intSetting("history_sample_interval_ms", kDefaultHistorySampleIntervalMs, 100,
                      60000);
}

int ConfigManager::retentionDays() const {
    return intSetting("retention_days", kDefaultRetentionDays, 1, 365);
}

QString ConfigManager::theme() const {
    const std::string raw = setting("theme");
    const QString value = QString::fromStdString(raw).toLower();
    return value == QLatin1String("light") ? QStringLiteral("light") : QStringLiteral("dark");
}

QString ConfigManager::networkUnit() const {
    const std::string raw = setting("network_unit");
    const QString value = QString::fromStdString(raw).toLower();
    return value == QLatin1String("bits") ? QStringLiteral("bits") : QStringLiteral("bytes");
}

bool ConfigManager::collectorCpu() const {
    return boolSetting("collector_cpu", true);
}

bool ConfigManager::collectorMemory() const {
    return boolSetting("collector_memory", true);
}

bool ConfigManager::collectorDisk() const {
    return boolSetting("collector_disk", true);
}

bool ConfigManager::collectorNetwork() const {
    return boolSetting("collector_network", true);
}

void ConfigManager::writeSetting(const QString& key, const QString& value) {
    values_[key.toStdString()] = value.toStdString();
    if (database_ != nullptr) {
        QMetaObject::invokeMethod(database_, "setSetting", Qt::QueuedConnection,
                                  Q_ARG(QString, key), Q_ARG(QString, value));
    }
}

void ConfigManager::setIntervalMs(int value) {
    const int clamped = std::clamp(value, 100, 10000);
    writeSetting(QStringLiteral("interval_ms"), QString::number(clamped));
    emit intervalChanged(clamped);
}

void ConfigManager::setHistoryEnabled(bool value) {
    writeSetting(QStringLiteral("history_enabled"), value ? QStringLiteral("true")
                                                          : QStringLiteral("false"));
    emit historyEnabledChanged(value);
}

void ConfigManager::setHistorySampleIntervalMs(int value) {
    const int clamped = std::clamp(value, 100, 60000);
    writeSetting(QStringLiteral("history_sample_interval_ms"), QString::number(clamped));
    emit historySampleIntervalChanged(clamped);
}

void ConfigManager::setRetentionDays(int value) {
    const int clamped = std::clamp(value, 1, 365);
    writeSetting(QStringLiteral("retention_days"), QString::number(clamped));
    emit retentionDaysChanged(clamped);
}

void ConfigManager::setTheme(const QString& value) {
    const QString normalized = value.toLower() == QLatin1String("light")
                                   ? QStringLiteral("light")
                                   : QStringLiteral("dark");
    writeSetting(QStringLiteral("theme"), normalized);
    emit themeChanged(normalized);
}

void ConfigManager::setNetworkUnit(const QString& value) {
    const QString normalized = value.toLower() == QLatin1String("bits")
                                   ? QStringLiteral("bits")
                                   : QStringLiteral("bytes");
    writeSetting(QStringLiteral("network_unit"), normalized);
    emit networkUnitChanged(normalized);
}

void ConfigManager::setCollectorCpu(bool value) {
    writeSetting(QStringLiteral("collector_cpu"), value ? QStringLiteral("true")
                                                        : QStringLiteral("false"));
    emit collectorCpuChanged(value);
}

void ConfigManager::setCollectorMemory(bool value) {
    writeSetting(QStringLiteral("collector_memory"), value ? QStringLiteral("true")
                                                           : QStringLiteral("false"));
    emit collectorMemoryChanged(value);
}

void ConfigManager::setCollectorDisk(bool value) {
    writeSetting(QStringLiteral("collector_disk"), value ? QStringLiteral("true")
                                                         : QStringLiteral("false"));
    emit collectorDiskChanged(value);
}

void ConfigManager::setCollectorNetwork(bool value) {
    writeSetting(QStringLiteral("collector_network"), value ? QStringLiteral("true")
                                                            : QStringLiteral("false"));
    emit collectorNetworkChanged(value);
}

void ConfigManager::onAllSettings(const QVector<AppSetting>& settings) {
    values_.clear();
    for (const AppSetting& item : settings) {
        values_[item.key] = item.value;
    }
    emit loaded();
}

void ConfigManager::onSettingReady(const QString& key, const QString& value) {
    values_[key.toStdString()] = value.toStdString();
}

} // namespace srm
