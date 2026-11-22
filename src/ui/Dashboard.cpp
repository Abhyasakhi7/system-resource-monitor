#include "ui/Dashboard.h"
#include "ui/ChartUtils.h"
#include "ui/FormatUtils.h"
#include "ui/MetricCard.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

#include <QColor>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace srm {

Dashboard::Dashboard(QWidget* parent)
    : QWidget(parent), cpuHistory_(kMaxSamples), memoryHistory_(kMaxSamples) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(12);

    cpuCard_ = new MetricCard(QStringLiteral("CPU"), this);
    memoryCard_ = new MetricCard(QStringLiteral("Memory"), this);
    diskCard_ = new MetricCard(QStringLiteral("Disk"), this);
    networkCard_ = new MetricCard(QStringLiteral("Network"), this);

    cardsLayout->addWidget(cpuCard_);
    cardsLayout->addWidget(memoryCard_);
    cardsLayout->addWidget(diskCard_);
    cardsLayout->addWidget(networkCard_);
    layout->addLayout(cardsLayout);

    auto* chartsLayout = new QHBoxLayout();
    chartsLayout->setSpacing(12);

    cpuChartView_ = ui::createLineChart(cpuSeries_, QStringLiteral("CPU Usage"), 0.0, 100.0,
                                       static_cast<int>(kMaxSamples), QColor("#4f8cff"));
    memoryChartView_ = ui::createLineChart(memorySeries_, QStringLiteral("Memory Usage"), 0.0,
                                          100.0, static_cast<int>(kMaxSamples),
                                          QColor("#4fd0a0"));

    chartsLayout->addWidget(cpuChartView_);
    chartsLayout->addWidget(memoryChartView_);
    layout->addLayout(chartsLayout, 1);
}

void Dashboard::setNetworkUnit(const QString& unit) {
    networkBits_ = unit.toLower() == QLatin1String("bits");
}

void Dashboard::appendSample(QLineSeries* series, MetricRingBuffer& history, double value) {
    if (series == nullptr) {
        return;
    }
    history.append(value);

    series->clear();
    const auto& values = history.values();
    for (std::size_t i = 0; i < values.size(); ++i) {
        series->append(static_cast<qreal>(i), values[i]);
    }
}

void Dashboard::updateSnapshot(const SystemMetricsSnapshot& snapshot) {
    if (snapshot.cpuAvailable && snapshot.cpu.totalUsagePercent.has_value()) {
        cpuCard_->setValue(ui::formatPercent(*snapshot.cpu.totalUsagePercent));

        QString subtitle = QStringLiteral("%1 cores")
                               .arg(snapshot.cpu.perCoreUsagePercent.size());
        if (snapshot.cpu.frequencyMhz.has_value()) {
            subtitle += QStringLiteral(" · %1 MHz")
                            .arg(*snapshot.cpu.frequencyMhz, 0, 'f', 0);
        }
        cpuCard_->setSubtitle(subtitle);
        appendSample(cpuSeries_, cpuHistory_, *snapshot.cpu.totalUsagePercent);
    } else {
        cpuCard_->setValue(QStringLiteral("—"));
        cpuCard_->setSubtitle(snapshot.cpuAvailable ? QStringLiteral("Collecting…")
                                                    : QStringLiteral("Unavailable"));
    }

    if (snapshot.memoryAvailable && snapshot.memory.usedBytes.has_value() &&
        snapshot.memory.totalBytes.has_value() && *snapshot.memory.totalBytes > 0) {
        const double usedPercent = 100.0 * static_cast<double>(*snapshot.memory.usedBytes) /
                                   static_cast<double>(*snapshot.memory.totalBytes);
        memoryCard_->setValue(ui::formatBytes(*snapshot.memory.usedBytes) +
                              QStringLiteral(" / ") +
                              ui::formatBytes(*snapshot.memory.totalBytes));
        memoryCard_->setSubtitle(
            QStringLiteral("%1 used").arg(ui::formatPercent(usedPercent)));
        appendSample(memorySeries_, memoryHistory_, usedPercent);
    } else {
        memoryCard_->setValue(QStringLiteral("—"));
        memoryCard_->setSubtitle(snapshot.memoryAvailable ? QStringLiteral("Collecting…")
                                                          : QStringLiteral("Unavailable"));
    }

    if (snapshot.diskAvailable) {
        const QString read =
            snapshot.disk.readBytesPerSec.has_value()
                ? ui::formatRate(*snapshot.disk.readBytesPerSec)
                : QStringLiteral("—");
        const QString write =
            snapshot.disk.writeBytesPerSec.has_value()
                ? ui::formatRate(*snapshot.disk.writeBytesPerSec)
                : QStringLiteral("—");
        diskCard_->setValue(read + QStringLiteral(" / ") + write);

        QString subtitle = QStringLiteral("Read / Write");
        if (!snapshot.disk.filesystems.empty()) {
            const FilesystemMetrics& filesystem = snapshot.disk.filesystems.front();
            if (filesystem.freeBytes.has_value() && filesystem.totalBytes.has_value()) {
                subtitle = ui::formatBytes(*filesystem.freeBytes) +
                           QStringLiteral(" free of ") +
                           ui::formatBytes(*filesystem.totalBytes) + QStringLiteral(" · ") +
                           QString::fromStdString(filesystem.mountPoint);
            }
        }
        diskCard_->setSubtitle(subtitle);
    } else {
        diskCard_->setValue(QStringLiteral("—"));
        diskCard_->setSubtitle(QStringLiteral("Unavailable"));
    }

    if (snapshot.networkAvailable) {
        const QString download =
            snapshot.network.totalRxBytesPerSec.has_value()
                ? ui::formatRate(*snapshot.network.totalRxBytesPerSec, networkBits_)
                : QStringLiteral("—");
        const QString upload =
            snapshot.network.totalTxBytesPerSec.has_value()
                ? ui::formatRate(*snapshot.network.totalTxBytesPerSec, networkBits_)
                : QStringLiteral("—");
        networkCard_->setValue(download + QStringLiteral(" / ") + upload);
        networkCard_->setSubtitle(QStringLiteral("Download / Upload"));
    } else {
        networkCard_->setValue(QStringLiteral("—"));
        networkCard_->setSubtitle(QStringLiteral("Unavailable"));
    }
}

} // namespace srm
