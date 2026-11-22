#include "ui/SettingsPage.h"

#include "services/ConfigManager.h"
#include "services/HistoryService.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

namespace srm {

SettingsPage::SettingsPage(ConfigManager* config, HistoryService* history,
                           const QString& databasePath, QWidget* parent)
    : QWidget(parent), config_(config), history_(history), databasePath_(databasePath) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);

    tabs_ = new QTabWidget(this);
    layout->addWidget(tabs_, 1);

    QWidget* monitoringTab = new QWidget(tabs_);
    auto* monitoringForm = new QFormLayout(monitoringTab);

    intervalSpin_ = new QSpinBox(monitoringTab);
    intervalSpin_->setRange(100, 10000);
    intervalSpin_->setSingleStep(100);
    intervalSpin_->setSuffix(QStringLiteral(" ms"));
    monitoringForm->addRow(QStringLiteral("Refresh every"), intervalSpin_);

    cpuCheck_ = new QCheckBox(QStringLiteral("CPU"), monitoringTab);
    memoryCheck_ = new QCheckBox(QStringLiteral("Memory"), monitoringTab);
    diskCheck_ = new QCheckBox(QStringLiteral("Disk"), monitoringTab);
    networkCheck_ = new QCheckBox(QStringLiteral("Network"), monitoringTab);
    monitoringForm->addRow(QStringLiteral("Collectors"), cpuCheck_);
    monitoringForm->addRow(QString(), memoryCheck_);
    monitoringForm->addRow(QString(), diskCheck_);
    monitoringForm->addRow(QString(), networkCheck_);

    historyCheck_ = new QCheckBox(QStringLiteral("Store metric history"), monitoringTab);
    monitoringForm->addRow(QStringLiteral("History"), historyCheck_);

    historyIntervalSpin_ = new QSpinBox(monitoringTab);
    historyIntervalSpin_->setRange(100, 60000);
    historyIntervalSpin_->setSingleStep(100);
    historyIntervalSpin_->setSuffix(QStringLiteral(" ms"));
    monitoringForm->addRow(QStringLiteral("History sample interval"), historyIntervalSpin_);

    tabs_->addTab(monitoringTab, QStringLiteral("Monitoring"));

    QWidget* appearanceTab = new QWidget(tabs_);
    auto* appearanceForm = new QFormLayout(appearanceTab);

    themeCombo_ = new QComboBox(appearanceTab);
    themeCombo_->addItem(QStringLiteral("Dark"), QStringLiteral("dark"));
    themeCombo_->addItem(QStringLiteral("Light"), QStringLiteral("light"));
    appearanceForm->addRow(QStringLiteral("Theme"), themeCombo_);

    networkUnitCombo_ = new QComboBox(appearanceTab);
    networkUnitCombo_->addItem(QStringLiteral("Bytes/s"), QStringLiteral("bytes"));
    networkUnitCombo_->addItem(QStringLiteral("Bits/s"), QStringLiteral("bits"));
    appearanceForm->addRow(QStringLiteral("Network speed unit"), networkUnitCombo_);

    tabs_->addTab(appearanceTab, QStringLiteral("Appearance"));

    QWidget* historyTab = new QWidget(tabs_);
    auto* historyForm = new QFormLayout(historyTab);

    retentionSpin_ = new QSpinBox(historyTab);
    retentionSpin_->setRange(1, 365);
    retentionSpin_->setSuffix(QStringLiteral(" days"));
    historyForm->addRow(QStringLiteral("Retention"), retentionSpin_);

    databasePathLabel_ = new QLabel(databasePath_, historyTab);
    databasePathLabel_->setWordWrap(true);
    databasePathLabel_->setStyleSheet(QStringLiteral("color:#9aa0a6;"));
    historyForm->addRow(QStringLiteral("Database file"), databasePathLabel_);

    clearHistoryButton_ = new QPushButton(QStringLiteral("Clear history"), historyTab);
    historyForm->addRow(QString(), clearHistoryButton_);

    tabs_->addTab(historyTab, QStringLiteral("History"));

    QWidget* alertsTab = new QWidget(tabs_);
    auto* alertsForm = new QFormLayout(alertsTab);
    auto* alertsNote = new QLabel(
        QStringLiteral("Smart alerts are planned for a later phase."), alertsTab);
    alertsNote->setStyleSheet(QStringLiteral("color:#9aa0a6;"));
    alertsForm->addRow(alertsNote);
    auto* cpuThreshold = new QSpinBox(alertsTab);
    cpuThreshold->setRange(1, 100);
    cpuThreshold->setSuffix(QStringLiteral(" %"));
    cpuThreshold->setEnabled(false);
    alertsForm->addRow(QStringLiteral("CPU threshold"), cpuThreshold);
    auto* memoryThreshold = new QSpinBox(alertsTab);
    memoryThreshold->setRange(1, 100);
    memoryThreshold->setSuffix(QStringLiteral(" %"));
    memoryThreshold->setEnabled(false);
    alertsForm->addRow(QStringLiteral("Memory threshold"), memoryThreshold);
    tabs_->addTab(alertsTab, QStringLiteral("Alerts"));

    QWidget* benchmarkTab = new QWidget(tabs_);
    auto* benchmarkForm = new QFormLayout(benchmarkTab);
    auto* benchmarkNote =
        new QLabel(QStringLiteral("Benchmark is planned for a later phase."), benchmarkTab);
    benchmarkNote->setStyleSheet(QStringLiteral("color:#9aa0a6;"));
    benchmarkForm->addRow(benchmarkNote);
    auto* benchmarkDuration = new QSpinBox(benchmarkTab);
    benchmarkDuration->setRange(1, 120);
    benchmarkDuration->setSuffix(QStringLiteral(" s"));
    benchmarkDuration->setEnabled(false);
    benchmarkForm->addRow(QStringLiteral("Duration"), benchmarkDuration);
    tabs_->addTab(benchmarkTab, QStringLiteral("Benchmark"));

    applyInitialValues();

    connect(intervalSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int value) {
                if (config_ != nullptr) {
                    config_->setIntervalMs(value);
                }
            });
    connect(cpuCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        if (config_ != nullptr) {
            config_->setCollectorCpu(checked);
        }
    });
    connect(memoryCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        if (config_ != nullptr) {
            config_->setCollectorMemory(checked);
        }
    });
    connect(diskCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        if (config_ != nullptr) {
            config_->setCollectorDisk(checked);
        }
    });
    connect(networkCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        if (config_ != nullptr) {
            config_->setCollectorNetwork(checked);
        }
    });
    connect(historyCheck_, &QCheckBox::toggled, this, [this](bool checked) {
        if (config_ != nullptr) {
            config_->setHistoryEnabled(checked);
        }
    });
    connect(historyIntervalSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int value) {
                if (config_ != nullptr) {
                    config_->setHistorySampleIntervalMs(value);
                }
            });
    connect(themeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                if (config_ != nullptr) {
                    config_->setTheme(themeCombo_->itemData(index).toString());
                }
            });
    connect(networkUnitCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                if (config_ != nullptr) {
                    config_->setNetworkUnit(networkUnitCombo_->itemData(index).toString());
                }
            });
    connect(retentionSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int value) {
                if (config_ != nullptr) {
                    config_->setRetentionDays(value);
                }
                if (history_ != nullptr) {
                    history_->setRetentionDays(value);
                }
            });
    connect(clearHistoryButton_, &QPushButton::clicked, this,
            &SettingsPage::onClearHistory);
    if (config_ != nullptr) {
        connect(config_, &ConfigManager::loaded, this, &SettingsPage::onConfigLoaded);
    }
}

void SettingsPage::onConfigLoaded() {
    applyInitialValues();
}

void SettingsPage::applyInitialValues() {
    if (config_ == nullptr) {
        return;
    }

    const QSignalBlocker intervalBlocker(intervalSpin_);
    const QSignalBlocker cpuBlocker(cpuCheck_);
    const QSignalBlocker memoryBlocker(memoryCheck_);
    const QSignalBlocker diskBlocker(diskCheck_);
    const QSignalBlocker networkBlocker(networkCheck_);
    const QSignalBlocker historyBlocker(historyCheck_);
    const QSignalBlocker historyIntervalBlocker(historyIntervalSpin_);
    const QSignalBlocker retentionBlocker(retentionSpin_);
    const QSignalBlocker themeBlocker(themeCombo_);
    const QSignalBlocker unitBlocker(networkUnitCombo_);

    intervalSpin_->setValue(config_->intervalMs());
    cpuCheck_->setChecked(config_->collectorCpu());
    memoryCheck_->setChecked(config_->collectorMemory());
    diskCheck_->setChecked(config_->collectorDisk());
    networkCheck_->setChecked(config_->collectorNetwork());
    historyCheck_->setChecked(config_->historyEnabled());
    historyIntervalSpin_->setValue(config_->historySampleIntervalMs());
    retentionSpin_->setValue(config_->retentionDays());

    const QString theme = config_->theme();
    const int themeIndex = themeCombo_->findData(theme);
    if (themeIndex >= 0) {
        themeCombo_->setCurrentIndex(themeIndex);
    }

    const QString unit = config_->networkUnit();
    const int unitIndex = networkUnitCombo_->findData(unit);
    if (unitIndex >= 0) {
        networkUnitCombo_->setCurrentIndex(unitIndex);
    }
}

void SettingsPage::onClearHistory() {
    if (history_ == nullptr) {
        return;
    }
    const auto answer = QMessageBox::warning(
        this, QStringLiteral("Clear history"),
        QStringLiteral("Delete all stored metric history? This cannot be undone."),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer == QMessageBox::Yes) {
        history_->clearHistory();
    }
}

} // namespace srm
