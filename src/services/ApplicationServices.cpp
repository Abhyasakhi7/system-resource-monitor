#include "services/ApplicationServices.h"

#include "core/SystemMonitor.h"
#include "database/DatabaseManager.h"
#include "hardware/HardwareService.h"
#include "process/ProcessService.h"
#include "process/ProcessWorker.h"
#include "services/ConfigManager.h"
#include "services/HistoryService.h"

#include <QDir>
#include <QMetaObject>
#include <QStandardPaths>
#include <QThread>

namespace srm {

namespace {

constexpr int kDefaultRetentionDays = 7;

} // namespace

ApplicationServices::ApplicationServices(QObject* parent)
    : QObject(parent) {}

ApplicationServices::~ApplicationServices() {
    stopThreads();
}

void ApplicationServices::initialize() {
    const QString dataLocation =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataLocation);
    databasePath_ = dataLocation + QStringLiteral("/metrics.sqlite");

    monitorThread_ = new QThread(this);
    monitor_ = new SystemMonitor();
    monitor_->moveToThread(monitorThread_);
    connect(monitorThread_, &QThread::started, monitor_, &SystemMonitor::start);
    connect(monitor_, &SystemMonitor::stopped, monitorThread_, &QThread::quit);
    monitorThread_->start();

    databaseThread_ = new QThread(this);
    database_ = new DatabaseManager();
    database_->moveToThread(databaseThread_);
    databaseThread_->start();

    QMetaObject::invokeMethod(database_, "initialize", Qt::QueuedConnection,
                              Q_ARG(QString, databasePath_), Q_ARG(int, kDefaultRetentionDays));

    config_ = new ConfigManager(database_, this);
    history_ = new HistoryService(database_, config_, this);
    hardware_ = new HardwareService();
    processService_ = new ProcessService();

    config_->load();
}

void ApplicationServices::startProcessWorker() {
    if (processWorker_ != nullptr && processThread_ != nullptr &&
        processThread_->isRunning()) {
        return;
    }

    if (processThread_ == nullptr) {
        processThread_ = new QThread(this);
        processWorker_ = new ProcessWorker();
        processWorker_->moveToThread(processThread_);
        connect(processThread_, &QThread::started, processWorker_, &ProcessWorker::start);
        connect(processWorker_, &ProcessWorker::stopped, processThread_, &QThread::quit);
    }

    processThread_->start();
}

void ApplicationServices::stopProcessWorker() {
    if (processThread_ == nullptr) {
        return;
    }

    if (processThread_->isRunning()) {
        if (processWorker_ != nullptr) {
            QMetaObject::invokeMethod(processWorker_, "stop", Qt::BlockingQueuedConnection);
        }
        processThread_->quit();
        if (!processThread_->wait(3000)) {
            processThread_->terminate();
            processThread_->wait();
        }
    }

    delete processWorker_;
    processWorker_ = nullptr;
    delete processThread_;
    processThread_ = nullptr;
}

void ApplicationServices::stopThreads() {
    if (monitorThread_ != nullptr && monitorThread_->isRunning()) {
        if (monitor_ != nullptr) {
            QMetaObject::invokeMethod(monitor_, "stop", Qt::BlockingQueuedConnection);
        }
        monitorThread_->quit();
        if (!monitorThread_->wait(3000)) {
            monitorThread_->terminate();
            monitorThread_->wait();
        }
    }
    delete monitor_;
    monitor_ = nullptr;
    delete monitorThread_;
    monitorThread_ = nullptr;

    stopProcessWorker();

    delete hardware_;
    hardware_ = nullptr;
    delete processService_;
    processService_ = nullptr;

    if (databaseThread_ != nullptr) {
        if (databaseThread_->isRunning()) {
            databaseThread_->quit();
            if (!databaseThread_->wait(3000)) {
                databaseThread_->terminate();
                databaseThread_->wait();
            }
        }
        delete database_;
        database_ = nullptr;
        delete databaseThread_;
        databaseThread_ = nullptr;
    }
}

} // namespace srm
