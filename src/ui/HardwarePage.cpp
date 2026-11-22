#include "ui/HardwarePage.h"
#include "ui/FormatUtils.h"

#include "hardware/HardwareService.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

namespace srm {

namespace {

QString boolStatus(bool available) {
    return available ? QStringLiteral("Available") : QStringLiteral("Unavailable");
}

void clearTable(QTableWidget* table) {
    table->setRowCount(0);
}

} // namespace

HardwarePage::HardwarePage(HardwareService* service, QWidget* parent)
    : QWidget(parent), service_(service) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    refreshButton_ = new QPushButton(QStringLiteral("Refresh"), this);
    layout->addWidget(refreshButton_, 0, Qt::AlignLeft);

    tabs_ = new QTabWidget(this);
    buildTabs();
    layout->addWidget(tabs_, 1);

    connect(refreshButton_, &QPushButton::clicked, this, &HardwarePage::refresh);
}

void HardwarePage::buildTabs() {
    const auto makeTable = [this]() {
        auto* table = new QTableWidget(tabs_);
        table->setColumnCount(4);
        table->setHorizontalHeaderLabels(
            {QStringLiteral("Property"), QStringLiteral("Value"), QStringLiteral("Status"),
             QStringLiteral("Source")});
        table->horizontalHeader()->setStretchLastSection(true);
        table->setSelectionMode(QAbstractItemView::NoSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        return table;
    };

    overviewTable_ = makeTable();
    processorTable_ = makeTable();
    systemTable_ = makeTable();
    memoryTable_ = makeTable();
    graphicsTable_ = makeTable();
    storageTable_ = makeTable();
    motherboardTable_ = makeTable();
    batteryTable_ = makeTable();

    tabs_->addTab(overviewTable_, QStringLiteral("Overview"));
    tabs_->addTab(processorTable_, QStringLiteral("Processor"));
    tabs_->addTab(systemTable_, QStringLiteral("System"));
    tabs_->addTab(memoryTable_, QStringLiteral("Memory"));
    tabs_->addTab(graphicsTable_, QStringLiteral("Graphics"));
    tabs_->addTab(storageTable_, QStringLiteral("Storage"));
    tabs_->addTab(motherboardTable_, QStringLiteral("Motherboard"));
    tabs_->addTab(batteryTable_, QStringLiteral("Battery"));
}

void HardwarePage::addRow(QTableWidget* table, const QString& label, const QString& value,
                          const QString& status, const QString& source) {
    if (table == nullptr) {
        return;
    }
    const int row = table->rowCount();
    table->insertRow(row);
    table->setItem(row, 0, new QTableWidgetItem(label));
    table->setItem(row, 1, new QTableWidgetItem(value));
    table->setItem(row, 2, new QTableWidgetItem(status));
    table->setItem(row, 3, new QTableWidgetItem(source));
}

void HardwarePage::refresh() {
    if (service_ != nullptr) {
        updateReport(service_->collect());
    }
}

void HardwarePage::populate(const HardwareReport& report) {
    clearTable(overviewTable_);
    clearTable(processorTable_);
    clearTable(systemTable_);
    clearTable(memoryTable_);
    clearTable(graphicsTable_);
    clearTable(storageTable_);
    clearTable(motherboardTable_);
    clearTable(batteryTable_);

    addRow(processorTable_, QStringLiteral("Model"), QString::fromStdString(report.cpu.model),
           boolStatus(report.cpu.available), QString::fromStdString(report.cpu.source));
    addRow(processorTable_, QStringLiteral("Vendor"), QString::fromStdString(report.cpu.vendor));
    addRow(processorTable_, QStringLiteral("Architecture"),
           QString::fromStdString(report.cpu.architecture));
    addRow(processorTable_, QStringLiteral("Physical cores"),
           QString::number(report.cpu.physicalCores));
    addRow(processorTable_, QStringLiteral("Logical threads"),
           QString::number(report.cpu.logicalThreads));
    addRow(processorTable_, QStringLiteral("Frequency"),
           report.cpu.frequencyMhz.has_value()
               ? QStringLiteral("%1 MHz").arg(*report.cpu.frequencyMhz, 0, 'f', 0)
               : QStringLiteral("Unavailable"));

    addRow(systemTable_, QStringLiteral("Hostname"),
           QString::fromStdString(report.system.hostname));
    addRow(systemTable_, QStringLiteral("Operating system"),
           QString::fromStdString(report.system.osPrettyName), boolStatus(report.system.available),
           QString::fromStdString(report.system.source));
    addRow(systemTable_, QStringLiteral("Kernel"),
           QString::fromStdString(report.system.kernelVersion));
    addRow(systemTable_, QStringLiteral("Architecture"),
           QString::fromStdString(report.system.architecture));

    addRow(memoryTable_, QStringLiteral("Total capacity"),
           report.memory.totalBytes.has_value()
               ? ui::formatBytes(*report.memory.totalBytes)
               : QStringLiteral("Unavailable"),
           boolStatus(report.memory.available), QString::fromStdString(report.memory.source));
    addRow(memoryTable_, QStringLiteral("Modules"), QStringLiteral("Unavailable without root"),
           QStringLiteral("Permission denied"), QStringLiteral("/sys/devices/system/memory"));

    addRow(graphicsTable_, QStringLiteral("Device"),
           report.gpu.available ? QString::fromStdString(report.gpu.name)
                                : QStringLiteral("Unavailable"),
           boolStatus(report.gpu.available), QString::fromStdString(report.gpu.source));
    addRow(graphicsTable_, QStringLiteral("Vendor ID"),
           QString::fromStdString(report.gpu.vendorId));
    addRow(graphicsTable_, QStringLiteral("Device ID"),
           QString::fromStdString(report.gpu.deviceId));
    addRow(graphicsTable_, QStringLiteral("Driver"), QString::fromStdString(report.gpu.driver));
    addRow(graphicsTable_, QStringLiteral("VRAM"),
           report.gpu.vramBytes.has_value() ? ui::formatBytes(*report.gpu.vramBytes)
                                            : QStringLiteral("Unavailable"));

    if (report.storage.devices.empty()) {
        addRow(storageTable_, QStringLiteral("Devices"), QStringLiteral("Unavailable"),
               QStringLiteral("Unavailable"), QString());
    } else {
        for (const StorageDeviceInformation& device : report.storage.devices) {
            addRow(storageTable_, QString::fromStdString(device.mountPoint),
                   QString::fromStdString(device.device),
                   device.totalBytes.has_value() ? ui::formatBytes(*device.totalBytes)
                                                 : QStringLiteral("Unavailable"),
                   QString::fromStdString(device.filesystem));
            addRow(storageTable_, QStringLiteral("  Model"),
                   QString::fromStdString(device.model));
        }
    }
    addRow(storageTable_, QStringLiteral("Source"),
           QString::fromStdString(report.storage.source));

    addRow(motherboardTable_, QStringLiteral("Manufacturer"),
           QString::fromStdString(report.motherboard.manufacturer),
           boolStatus(report.motherboard.available),
           QString::fromStdString(report.motherboard.source));
    addRow(motherboardTable_, QStringLiteral("Model"),
           QString::fromStdString(report.motherboard.model));
    addRow(motherboardTable_, QStringLiteral("Product"),
           QString::fromStdString(report.motherboard.productName));

    addRow(batteryTable_, QStringLiteral("Name"),
           report.battery.available ? QString::fromStdString(report.battery.name)
                                    : QStringLiteral("Unavailable"),
           boolStatus(report.battery.available), QString::fromStdString(report.battery.source));
    addRow(batteryTable_, QStringLiteral("Capacity"),
           report.battery.capacityPercent.has_value()
               ? QStringLiteral("%1 %").arg(*report.battery.capacityPercent)
               : QStringLiteral("Unavailable"));
    addRow(batteryTable_, QStringLiteral("Status"),
           QString::fromStdString(report.battery.status));
    addRow(batteryTable_, QStringLiteral("Health"), QString::fromStdString(report.battery.health));

    addRow(overviewTable_, QStringLiteral("Processor"),
           QString::fromStdString(report.cpu.model), boolStatus(report.cpu.available),
           QString::fromStdString(report.cpu.source));
    addRow(overviewTable_, QStringLiteral("Memory"),
           report.memory.totalBytes.has_value()
               ? ui::formatBytes(*report.memory.totalBytes)
               : QStringLiteral("Unavailable"),
           boolStatus(report.memory.available), QString::fromStdString(report.memory.source));
    addRow(overviewTable_, QStringLiteral("Graphics"),
           report.gpu.available ? QString::fromStdString(report.gpu.name)
                                : QStringLiteral("Unavailable"),
           boolStatus(report.gpu.available), QString::fromStdString(report.gpu.source));
    addRow(overviewTable_, QStringLiteral("Operating system"),
           QString::fromStdString(report.system.osPrettyName), boolStatus(report.system.available),
           QString::fromStdString(report.system.source));
    addRow(overviewTable_, QStringLiteral("Kernel"),
           QString::fromStdString(report.system.kernelVersion));
    addRow(overviewTable_, QStringLiteral("Battery"),
           report.battery.available ? QString::fromStdString(report.battery.name)
                                    : QStringLiteral("Unavailable"),
           boolStatus(report.battery.available), QString::fromStdString(report.battery.source));
    addRow(overviewTable_, QStringLiteral("Storage devices"),
           QString::number(report.storage.devices.size()), boolStatus(report.storage.available),
           QString::fromStdString(report.storage.source));

    for (QTableWidget* table : {overviewTable_, processorTable_, systemTable_, memoryTable_,
                                graphicsTable_, storageTable_, motherboardTable_,
                                batteryTable_}) {
        table->resizeColumnsToContents();
        table->horizontalHeader()->setStretchLastSection(true);
    }
}

void HardwarePage::updateReport(const HardwareReport& report) {
    populate(report);
}

} // namespace srm
