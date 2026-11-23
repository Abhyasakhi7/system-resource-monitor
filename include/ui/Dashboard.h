#pragma once

#include "core/MetricHistory.h"
#include "core/Metrics.h"

#include <QVector>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QChartView;
class QLineSeries;
QT_END_NAMESPACE

namespace srm {

class MetricCard;

class Dashboard : public QWidget {
    Q_OBJECT

public:
    explicit Dashboard(QWidget* parent = nullptr);

public slots:
    void updateSnapshot(const srm::SystemMetricsSnapshot& snapshot);
    void setNetworkUnit(const QString& unit);

private:
    void appendSample(QLineSeries* series, MetricRingBuffer& history, double value);

    MetricCard* cpuCard_{nullptr};
    MetricCard* memoryCard_{nullptr};
    MetricCard* diskCard_{nullptr};
    MetricCard* networkCard_{nullptr};
    MetricCard* temperatureCard_{nullptr};
    MetricCard* batteryCard_{nullptr};
    QChartView* cpuChartView_{nullptr};
    QChartView* memoryChartView_{nullptr};
    QLineSeries* cpuSeries_{nullptr};
    QLineSeries* memorySeries_{nullptr};
    MetricRingBuffer cpuHistory_;
    MetricRingBuffer memoryHistory_;
    bool networkBits_{false};

    static constexpr std::size_t kMaxSamples = 60;
};

} // namespace srm
