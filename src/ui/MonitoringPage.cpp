#include "ui/MonitoringPage.h"
#include "ui/ChartUtils.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <QColor>
#include <QGridLayout>

#include <cmath>

namespace srm {

MonitoringPage::MonitoringPage(QWidget* parent)
    : QWidget(parent), cpuHistory_(kMaxSamples), memoryHistory_(kMaxSamples),
      diskHistory_(kMaxSamples), networkHistory_(kMaxSamples) {
    auto* layout = new QGridLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    cpuChart_ = ui::createLineChart(cpuSeries_, QStringLiteral("CPU Usage"), 0.0, 100.0,
                                   static_cast<int>(kMaxSamples), QColor("#4f8cff"));
    memoryChart_ = ui::createLineChart(memorySeries_, QStringLiteral("Memory Usage"), 0.0,
                                      100.0, static_cast<int>(kMaxSamples),
                                      QColor("#4fd0a0"));
    diskChart_ = ui::createLineChart(diskSeries_, QStringLiteral("Disk Read"), 0.0, 100.0,
                                    static_cast<int>(kMaxSamples), QColor("#e0a04f"));
    networkChart_ = ui::createLineChart(networkSeries_, QStringLiteral("Network Download"),
                                       0.0, 100.0, static_cast<int>(kMaxSamples),
                                       QColor("#b06fd0"));

    layout->addWidget(cpuChart_, 0, 0);
    layout->addWidget(memoryChart_, 0, 1);
    layout->addWidget(diskChart_, 1, 0);
    layout->addWidget(networkChart_, 1, 1);
}

void MonitoringPage::setNetworkUnit(const QString& unit) {
    networkBits_ = unit.toLower() == QLatin1String("bits");
}

void MonitoringPage::appendSample(QLineSeries* series, MetricRingBuffer& history,
                                  double value) {
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

void MonitoringPage::updateAxis(QChartView* view, double latest) {
    if (view == nullptr) {
        return;
    }
    const QList<QAbstractAxis*> axes = view->chart()->axes(Qt::Vertical);
    if (axes.isEmpty()) {
        return;
    }
    auto* axis = qobject_cast<QValueAxis*>(axes.first());
    if (axis == nullptr) {
        return;
    }
    const double maximum = std::max(1.0, latest * 1.2);
    axis->setRange(0.0, maximum);
}

void MonitoringPage::updateSnapshot(const SystemMetricsSnapshot& snapshot) {
    if (snapshot.cpuAvailable && snapshot.cpu.totalUsagePercent.has_value()) {
        appendSample(cpuSeries_, cpuHistory_, *snapshot.cpu.totalUsagePercent);
    }

    if (snapshot.memoryAvailable && snapshot.memory.usedBytes.has_value() &&
        snapshot.memory.totalBytes.has_value() && *snapshot.memory.totalBytes > 0) {
        const double usedPercent = 100.0 * static_cast<double>(*snapshot.memory.usedBytes) /
                                   static_cast<double>(*snapshot.memory.totalBytes);
        appendSample(memorySeries_, memoryHistory_, usedPercent);
    }

    if (snapshot.diskAvailable && snapshot.disk.readBytesPerSec.has_value()) {
        const double value = *snapshot.disk.readBytesPerSec;
        appendSample(diskSeries_, diskHistory_, value);
        updateAxis(diskChart_, value);
    }

    if (snapshot.networkAvailable && snapshot.network.totalRxBytesPerSec.has_value()) {
        double value = *snapshot.network.totalRxBytesPerSec;
        if (networkBits_) {
            value *= 8.0;
        }
        appendSample(networkSeries_, networkHistory_, value);
        updateAxis(networkChart_, value);
    }
}

} // namespace srm
