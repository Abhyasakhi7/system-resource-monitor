#include "ui/MainWindow.h"

#include "services/ApplicationServices.h"
#include "services/ConfigManager.h"
#include "services/HistoryService.h"
#include "core/SystemMonitor.h"
#include "process/ProcessWorker.h"
#include "ui/AnalyticsPage.h"
#include "ui/Dashboard.h"
#include "ui/HardwarePage.h"
#include "ui/MonitoringPage.h"
#include "ui/PlaceholderPage.h"
#include "ui/ProcessPage.h"
#include "ui/SettingsPage.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QMetaObject>
#include <QStackedWidget>

namespace srm {

MainWindow::MainWindow(ApplicationServices* services, QWidget* parent)
    : QMainWindow(parent), services_(services) {
    setWindowTitle(tr("System Resource Monitor"));

    auto* central = new QWidget(this);
    auto* rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    sidebar_ = new QListWidget(central);
    sidebar_->addItem(tr("Overview"));
    sidebar_->addItem(tr("Monitoring"));
    sidebar_->addItem(tr("Processes"));
    sidebar_->addItem(tr("Hardware"));
    sidebar_->addItem(tr("Analytics"));
    sidebar_->addItem(tr("Benchmark"));
    sidebar_->addItem(tr("System Tools"));
    sidebar_->addItem(tr("Settings"));
    sidebar_->setFixedWidth(180);
    rootLayout->addWidget(sidebar_);

    stack_ = new QStackedWidget(central);
    dashboard_ = new Dashboard(stack_);
    monitoringPage_ = new MonitoringPage(stack_);
    processPage_ = new ProcessPage(services->processService(), stack_);
    hardwarePage_ = new HardwarePage(services->hardware(), stack_);
    analyticsPage_ = new AnalyticsPage(services->history(), stack_);
    auto* benchmarkPage = new PlaceholderPage(
        QStringLiteral("Benchmark"),
        QStringLiteral("CPU, memory, disk, and system benchmarks are planned for a later "
                       "phase."),
        stack_);
    auto* systemToolsPage = new PlaceholderPage(
        QStringLiteral("System Tools"),
        QStringLiteral("Service, startup, disk, network, and system information are planned "
                       "for a later phase."),
        stack_);
    settingsPage_ = new SettingsPage(services->config(), services->history(),
                                     services->databasePath(), stack_);

    stack_->addWidget(dashboard_);
    stack_->addWidget(monitoringPage_);
    stack_->addWidget(processPage_);
    stack_->addWidget(hardwarePage_);
    stack_->addWidget(analyticsPage_);
    stack_->addWidget(benchmarkPage);
    stack_->addWidget(systemToolsPage);
    stack_->addWidget(settingsPage_);
    rootLayout->addWidget(stack_, 1);

    setCentralWidget(central);

    connect(sidebar_, &QListWidget::currentRowChanged, stack_,
            &QStackedWidget::setCurrentIndex);
    connect(sidebar_, &QListWidget::currentRowChanged, this, &MainWindow::onPageChanged);

    connectServices();
    sidebar_->setCurrentRow(0);
}

MainWindow::~MainWindow() {
    if (services_ != nullptr) {
        services_->stopProcessWorker();
    }
}

void MainWindow::connectServices() {
    SystemMonitor* monitor = services_->monitor();
    if (monitor != nullptr) {
        connect(monitor, &SystemMonitor::snapshotReady, dashboard_,
                &Dashboard::updateSnapshot, Qt::QueuedConnection);
        connect(monitor, &SystemMonitor::snapshotReady, monitoringPage_,
                &MonitoringPage::updateSnapshot, Qt::QueuedConnection);
        connect(monitor, &SystemMonitor::snapshotReady, services_->history(),
                &HistoryService::recordSnapshot, Qt::QueuedConnection);
    }

    ConfigManager* config = services_->config();
    if (config != nullptr) {
        connect(config, &ConfigManager::loaded, this, &MainWindow::applyConfig);
        connect(config, &ConfigManager::intervalChanged, this, &MainWindow::applyConfig);
        connect(config, &ConfigManager::collectorCpuChanged, this,
                &MainWindow::applyConfig);
        connect(config, &ConfigManager::collectorMemoryChanged, this,
                &MainWindow::applyConfig);
        connect(config, &ConfigManager::collectorDiskChanged, this,
                &MainWindow::applyConfig);
        connect(config, &ConfigManager::collectorNetworkChanged, this,
                &MainWindow::applyConfig);
        connect(config, &ConfigManager::networkUnitChanged, this, &MainWindow::applyConfig);
    }

    if (services_->history() != nullptr && config != nullptr) {
        connect(config, &ConfigManager::retentionDaysChanged, services_->history(),
                &HistoryService::setRetentionDays);
    }

    applyConfig();
}

void MainWindow::applyConfig() {
    ConfigManager* config = services_->config();
    if (config == nullptr) {
        return;
    }

    if (services_->monitor() != nullptr) {
        QMetaObject::invokeMethod(services_->monitor(), "setInterval", Qt::QueuedConnection,
                                  Q_ARG(int, config->intervalMs()));
        QMetaObject::invokeMethod(
            services_->monitor(), "setCollectorEnabled", Qt::QueuedConnection,
            Q_ARG(bool, config->collectorCpu()), Q_ARG(bool, config->collectorMemory()),
            Q_ARG(bool, config->collectorDisk()), Q_ARG(bool, config->collectorNetwork()));
    }

    if (dashboard_ != nullptr) {
        dashboard_->setNetworkUnit(config->networkUnit());
    }
    if (monitoringPage_ != nullptr) {
        monitoringPage_->setNetworkUnit(config->networkUnit());
    }
}

void MainWindow::onPageChanged(int index) {
    if (index == processPageIndex_) {
        services_->startProcessWorker();
        ProcessWorker* worker = services_->processWorker();
        if (worker != nullptr) {
            connect(worker, &ProcessWorker::processesReady, processPage_,
                    &ProcessPage::updateProcesses, Qt::QueuedConnection);
        }
    } else {
        services_->stopProcessWorker();
    }

    if (index == hardwarePageIndex_) {
        hardwarePage_->refresh();
    }

    if (index == analyticsPageIndex_) {
        analyticsPage_->refresh();
    }
}

} // namespace srm
