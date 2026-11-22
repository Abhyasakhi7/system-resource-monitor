#include "database/DatabaseTypes.h"
#include "process/ProcessWorker.h"
#include "services/ApplicationServices.h"
#include "services/ConfigManager.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QColor>
#include <QMetaType>
#include <QPalette>
#include <QStyleFactory>

namespace {

void applyDarkTheme(QApplication& app) {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#1e1f22"));
    palette.setColor(QPalette::WindowText, QColor("#e6e6e6"));
    palette.setColor(QPalette::Base, QColor("#25262b"));
    palette.setColor(QPalette::AlternateBase, QColor("#2a2b30"));
    palette.setColor(QPalette::ToolTipBase, QColor("#2a2b30"));
    palette.setColor(QPalette::ToolTipText, QColor("#e6e6e6"));
    palette.setColor(QPalette::Text, QColor("#e6e6e6"));
    palette.setColor(QPalette::Button, QColor("#2a2b30"));
    palette.setColor(QPalette::ButtonText, QColor("#e6e6e6"));
    palette.setColor(QPalette::BrightText, QColor("#ffffff"));
    palette.setColor(QPalette::Highlight, QColor("#3a6ff0"));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    app.setPalette(palette);

    app.setStyleSheet(QStringLiteral(R"(
        QMainWindow, QWidget { background-color: #1e1f22; color: #e6e6e6; }
        QListWidget { background-color: #232428; border: none; outline: none; }
        QListWidget::item { padding: 12px 16px; border-radius: 6px; margin: 4px 8px; }
        QListWidget::item:selected { background-color: #3a3b40; }
        QListWidget::item:hover { background-color: #2e2f34; }
        QGroupBox { border: 1px solid #3a3b40; border-radius: 8px; margin-top: 12px;
                    padding-top: 8px; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px;
                           color: #c9cdd4; }
        QSpinBox { background-color: #2a2b30; border: 1px solid #3a3b40;
                   border-radius: 6px; padding: 6px; color: #e6e6e6; }
    )"));
}

void applyLightTheme(QApplication& app) {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#f5f6f7"));
    palette.setColor(QPalette::WindowText, QColor("#1f2328"));
    palette.setColor(QPalette::Base, QColor("#ffffff"));
    palette.setColor(QPalette::AlternateBase, QColor("#eef0f2"));
    palette.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
    palette.setColor(QPalette::ToolTipText, QColor("#1f2328"));
    palette.setColor(QPalette::Text, QColor("#1f2328"));
    palette.setColor(QPalette::Button, QColor("#e8eaed"));
    palette.setColor(QPalette::ButtonText, QColor("#1f2328"));
    palette.setColor(QPalette::BrightText, QColor("#000000"));
    palette.setColor(QPalette::Highlight, QColor("#3a6ff0"));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    app.setPalette(palette);

    app.setStyleSheet(QStringLiteral(R"(
        QMainWindow, QWidget { background-color: #f5f6f7; color: #1f2328; }
        QListWidget { background-color: #ffffff; border: none; outline: none; }
        QListWidget::item { padding: 12px 16px; border-radius: 6px; margin: 4px 8px; }
        QListWidget::item:selected { background-color: #d8e2f8; }
        QListWidget::item:hover { background-color: #eceef1; }
        QGroupBox { border: 1px solid #d0d3d8; border-radius: 8px; margin-top: 12px;
                    padding-top: 8px; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px;
                           color: #4a5058; }
        QSpinBox { background-color: #ffffff; border: 1px solid #c7cbd1;
                   border-radius: 6px; padding: 6px; color: #1f2328; }
    )"));
}

} // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName(QStringLiteral("System Resource Monitor"));
    app.setOrganizationName(QStringLiteral("SystemResourceMonitor"));

    qRegisterMetaType<srm::SystemMetricsSnapshot>("srm::SystemMetricsSnapshot");
    qRegisterMetaType<srm::MetricSample>("srm::MetricSample");
    qRegisterMetaType<srm::MetricStats>("srm::MetricStats");
    qRegisterMetaType<srm::TimeRange>("srm::TimeRange");
    qRegisterMetaType<srm::MetricKind>("srm::MetricKind");
    qRegisterMetaType<srm::AppSetting>("srm::AppSetting");
    qRegisterMetaType<QVector<srm::MetricSample>>("QVector<srm::MetricSample>");
    qRegisterMetaType<QVector<srm::AppSetting>>("QVector<srm::AppSetting>");
    qRegisterMetaType<srm::ProcessInfo>("srm::ProcessInfo");
    qRegisterMetaType<QVector<srm::ProcessInfo>>("QVector<srm::ProcessInfo>");

    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    applyDarkTheme(app);

    srm::ApplicationServices services;
    services.initialize();

    QObject::connect(services.config(), &srm::ConfigManager::themeChanged, &app,
                     [&app](const QString& theme) {
                         if (theme == QLatin1String("light")) {
                             applyLightTheme(app);
                         } else {
                             applyDarkTheme(app);
                         }
                     });

    srm::MainWindow window(&services);
    window.resize(1200, 760);
    window.show();

    return app.exec();
}
