#pragma once

#include <QWidget>

class QLabel;
class QCheckBox;
class QComboBox;
class QPushButton;
class QSpinBox;
class QTabWidget;

namespace srm {

class ConfigManager;
class HistoryService;

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    SettingsPage(ConfigManager* config, HistoryService* history,
                 const QString& databasePath, QWidget* parent = nullptr);

private slots:
    void onClearHistory();
    void onConfigLoaded();

private:
    void applyInitialValues();

    ConfigManager* config_{nullptr};
    HistoryService* history_{nullptr};
    QString databasePath_;
    QTabWidget* tabs_{nullptr};
    QSpinBox* intervalSpin_{nullptr};
    QCheckBox* cpuCheck_{nullptr};
    QCheckBox* memoryCheck_{nullptr};
    QCheckBox* diskCheck_{nullptr};
    QCheckBox* networkCheck_{nullptr};
    QCheckBox* temperatureCheck_{nullptr};
    QCheckBox* batteryCheck_{nullptr};
    QCheckBox* historyCheck_{nullptr};
    QSpinBox* historyIntervalSpin_{nullptr};
    QComboBox* themeCombo_{nullptr};
    QComboBox* networkUnitCombo_{nullptr};
    QSpinBox* retentionSpin_{nullptr};
    QLabel* databasePathLabel_{nullptr};
    QPushButton* clearHistoryButton_{nullptr};
};

} // namespace srm
