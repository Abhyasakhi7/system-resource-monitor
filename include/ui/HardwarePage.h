#pragma once

#include "hardware/HardwareInfo.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QPushButton;
class QTabWidget;
class QTableWidget;
QT_END_NAMESPACE

namespace srm {

class HardwareService;

class HardwarePage : public QWidget {
    Q_OBJECT

public:
    explicit HardwarePage(HardwareService* service, QWidget* parent = nullptr);

public slots:
    void refresh();
    void updateReport(const HardwareReport& report);

private:
    void buildTabs();
    void populate(const HardwareReport& report);
    void addRow(QTableWidget* table, const QString& label, const QString& value,
                const QString& status = QString(), const QString& source = QString());

    HardwareService* service_;
    QPushButton* refreshButton_{nullptr};
    QTabWidget* tabs_{nullptr};
    QTableWidget* overviewTable_{nullptr};
    QTableWidget* processorTable_{nullptr};
    QTableWidget* systemTable_{nullptr};
    QTableWidget* memoryTable_{nullptr};
    QTableWidget* graphicsTable_{nullptr};
    QTableWidget* storageTable_{nullptr};
    QTableWidget* motherboardTable_{nullptr};
    QTableWidget* batteryTable_{nullptr};
};

} // namespace srm
