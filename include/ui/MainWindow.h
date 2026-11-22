#pragma once

#include <QMainWindow>

class QListWidget;
class QStackedWidget;

namespace srm {

class AnalyticsPage;
class ApplicationServices;
class Dashboard;
class HardwarePage;
class MonitoringPage;
class ProcessPage;
class SettingsPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(ApplicationServices* services, QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onPageChanged(int index);
    void applyConfig();

private:
    void connectServices();

    ApplicationServices* services_{nullptr};
    QListWidget* sidebar_{nullptr};
    QStackedWidget* stack_{nullptr};
    Dashboard* dashboard_{nullptr};
    MonitoringPage* monitoringPage_{nullptr};
    ProcessPage* processPage_{nullptr};
    HardwarePage* hardwarePage_{nullptr};
    AnalyticsPage* analyticsPage_{nullptr};
    SettingsPage* settingsPage_{nullptr};
    int processPageIndex_{2};
    int hardwarePageIndex_{3};
    int analyticsPageIndex_{4};
};

} // namespace srm
