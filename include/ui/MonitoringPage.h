#pragma once

#include "core/MetricHistory.h"
#include "core/Metrics.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QChartView;
class QLabel;
class QLineSeries;
QT_END_NAMESPACE

namespace srm {

class MonitoringPage : public QWidget {
    Q_OBJECT

public:
    explicit MonitoringPage(QWidget* parent = nullptr);

public slots:
    void updateSnapshot(const srm::SystemMetricsSnapshot& snapshot);
    void setNetworkUnit(const QString& unit);

private:
    void appendSample(QLineSeries* series, MetricRingBuffer& history, double value);
    void updateAxis(QChartView* view, double latest);
    void updateSensorStatus(const srm::SystemMetricsSnapshot& snapshot);

    QChartView* cpuChart_{nullptr};
    QChartView* memoryChart_{nullptr};
    QChartView* diskChart_{nullptr};
    QChartView* networkChart_{nullptr};
    QChartView* temperatureChart_{nullptr};
    QChartView* batteryChart_{nullptr};
    QLabel* sensorStatusLabel_{nullptr};
    QLineSeries* cpuSeries_{nullptr};
    QLineSeries* memorySeries_{nullptr};
    QLineSeries* diskSeries_{nullptr};
    QLineSeries* networkSeries_{nullptr};
    QLineSeries* temperatureSeries_{nullptr};
    QLineSeries* batterySeries_{nullptr};
    MetricRingBuffer cpuHistory_;
    MetricRingBuffer memoryHistory_;
    MetricRingBuffer diskHistory_;
    MetricRingBuffer networkHistory_;
    MetricRingBuffer temperatureHistory_;
    MetricRingBuffer batteryHistory_;
    bool networkBits_{false};

    static constexpr std::size_t kMaxSamples = 120;
};

} // namespace srm
