#pragma once

#include "database/DatabaseTypes.h"

#include <QVector>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QChartView;
class QComboBox;
class QDateTimeEdit;
class QLabel;
class QLineSeries;
class QPushButton;
class QTableWidget;
QT_END_NAMESPACE

namespace srm {

class HistoryService;

class AnalyticsPage : public QWidget {
    Q_OBJECT

public:
    explicit AnalyticsPage(HistoryService* service, QWidget* parent = nullptr);

public slots:
    void refresh();
    void onSamplesReady(const QVector<MetricSample>& samples);
    void onStatsReady(MetricKind kind, const MetricStats& stats);
    void onExportFinished(bool ok, const QString& path, const QString& error);

private slots:
    void onRangeChanged(int index);
    void onQueryClicked();
    void onExportCsv();
    void onExportJson();

private:
    TimeRange currentRange() const;
    void populateChart();
    void populateTable();

    HistoryService* service_{nullptr};
    QComboBox* rangeCombo_{nullptr};
    QDateTimeEdit* fromEdit_{nullptr};
    QDateTimeEdit* toEdit_{nullptr};
    QPushButton* queryButton_{nullptr};
    QPushButton* exportCsvButton_{nullptr};
    QPushButton* exportJsonButton_{nullptr};
    QChartView* cpuChart_{nullptr};
    QChartView* memoryChart_{nullptr};
    QLineSeries* cpuSeries_{nullptr};
    QLineSeries* memorySeries_{nullptr};
    QLabel* cpuStatsLabel_{nullptr};
    QLabel* memoryStatsLabel_{nullptr};
    QTableWidget* table_{nullptr};
    QVector<MetricSample> samples_;
    MetricStats cpuStats_;
    MetricStats memoryStats_;
};

} // namespace srm
