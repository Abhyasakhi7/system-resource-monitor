#pragma once

#include "database/DatabaseManager.h"

#include <QObject>
#include <QString>

#include <map>
#include <string>

namespace srm {

class ConfigManager : public QObject {
    Q_OBJECT

public:
    explicit ConfigManager(DatabaseManager* database, QObject* parent = nullptr);

    void load();

    int intervalMs() const;
    bool historyEnabled() const;
    int historySampleIntervalMs() const;
    int retentionDays() const;
    QString theme() const;
    QString networkUnit() const;
    bool collectorCpu() const;
    bool collectorMemory() const;
    bool collectorDisk() const;
    bool collectorNetwork() const;
    bool collectorTemperature() const;
    bool collectorBattery() const;

    void setIntervalMs(int value);
    void setHistoryEnabled(bool value);
    void setHistorySampleIntervalMs(int value);
    void setRetentionDays(int value);
    void setTheme(const QString& value);
    void setNetworkUnit(const QString& value);
    void setCollectorCpu(bool value);
    void setCollectorMemory(bool value);
    void setCollectorDisk(bool value);
    void setCollectorNetwork(bool value);
    void setCollectorTemperature(bool value);
    void setCollectorBattery(bool value);

signals:
    void loaded();
    void intervalChanged(int value);
    void historyEnabledChanged(bool value);
    void historySampleIntervalChanged(int value);
    void retentionDaysChanged(int value);
    void themeChanged(const QString& value);
    void networkUnitChanged(const QString& value);
    void collectorCpuChanged(bool value);
    void collectorMemoryChanged(bool value);
    void collectorDiskChanged(bool value);
    void collectorNetworkChanged(bool value);
    void collectorTemperatureChanged(bool value);
    void collectorBatteryChanged(bool value);

private slots:
    void onAllSettings(const QVector<AppSetting>& settings);
    void onSettingReady(const QString& key, const QString& value);

private:
    std::string setting(const char* key) const;
    int intSetting(const char* key, int fallback, int minimum, int maximum) const;
    bool boolSetting(const char* key, bool fallback) const;
    void writeSetting(const QString& key, const QString& value);

    DatabaseManager* database_;
    std::map<std::string, std::string> values_;
};

} // namespace srm
