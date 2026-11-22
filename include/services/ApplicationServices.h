#pragma once

#include <QObject>
#include <QString>

class QThread;

namespace srm {

class ConfigManager;
class DatabaseManager;
class HardwareService;
class HistoryService;
class ProcessService;
class ProcessWorker;
class SystemMonitor;

class ApplicationServices : public QObject {
    Q_OBJECT

public:
    explicit ApplicationServices(QObject* parent = nullptr);
    ~ApplicationServices() override;

    void initialize();

    QString databasePath() const { return databasePath_; }

    ConfigManager* config() const { return config_; }
    DatabaseManager* database() const { return database_; }
    HistoryService* history() const { return history_; }
    HardwareService* hardware() const { return hardware_; }
    ProcessService* processService() const { return processService_; }
    ProcessWorker* processWorker() const { return processWorker_; }
    SystemMonitor* monitor() const { return monitor_; }

public slots:
    void startProcessWorker();
    void stopProcessWorker();

private:
    void stopThreads();

    QString databasePath_;
    QThread* monitorThread_{nullptr};
    SystemMonitor* monitor_{nullptr};
    QThread* databaseThread_{nullptr};
    DatabaseManager* database_{nullptr};
    ConfigManager* config_{nullptr};
    HistoryService* history_{nullptr};
    HardwareService* hardware_{nullptr};
    ProcessService* processService_{nullptr};
    QThread* processThread_{nullptr};
    ProcessWorker* processWorker_{nullptr};
};

} // namespace srm
