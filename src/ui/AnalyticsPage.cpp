#include "ui/AnalyticsPage.h"
#include "ui/ChartUtils.h"
#include "ui/FormatUtils.h"

#include "services/HistoryService.h"

#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>

#include <QComboBox>
#include <QAbstractItemView>
#include <QColor>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineSeries>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace srm {

namespace {

constexpr int kMaxChartPoints = 300;
constexpr int kMaxTableRows = 500;

double memoryPercent(const MetricSample& sample) {
    if (!sample.memoryUsedBytes || !sample.memoryTotalBytes || *sample.memoryTotalBytes == 0) {
        return 0.0;
    }
    return 100.0 * static_cast<double>(*sample.memoryUsedBytes) /
           static_cast<double>(*sample.memoryTotalBytes);
}

QString statsText(const QString& name, const MetricStats& stats, const QString& unit) {
    if (stats.count == 0 || !stats.average) {
        return QStringLiteral("%1: no data").arg(name);
    }
    return QStringLiteral("%1: min %2 %3, max %4 %3, avg %5 %3 (%6 samples)")
        .arg(name)
        .arg(*stats.minimum, 0, 'f', 1)
        .arg(unit)
        .arg(*stats.maximum, 0, 'f', 1)
        .arg(*stats.average, 0, 'f', 1)
        .arg(stats.count);
}

} // namespace

AnalyticsPage::AnalyticsPage(HistoryService* service, QWidget* parent)
    : QWidget(parent), service_(service) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto* controls = new QHBoxLayout();
    rangeCombo_ = new QComboBox(this);
    rangeCombo_->addItem(QStringLiteral("Last 1 hour"), QStringLiteral("1h"));
    rangeCombo_->addItem(QStringLiteral("Last 6 hours"), QStringLiteral("6h"));
    rangeCombo_->addItem(QStringLiteral("Last 12 hours"), QStringLiteral("12h"));
    rangeCombo_->addItem(QStringLiteral("Last 24 hours"), QStringLiteral("24h"));
    rangeCombo_->addItem(QStringLiteral("Last 7 days"), QStringLiteral("7d"));
    rangeCombo_->addItem(QStringLiteral("Custom"), QStringLiteral("custom"));
    controls->addWidget(rangeCombo_);

    fromEdit_ = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), this);
    fromEdit_->setCalendarPopup(true);
    fromEdit_->setVisible(false);
    toEdit_ = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    toEdit_->setCalendarPopup(true);
    toEdit_->setVisible(false);
    controls->addWidget(fromEdit_);
    controls->addWidget(toEdit_);

    queryButton_ = new QPushButton(QStringLiteral("Query"), this);
    exportCsvButton_ = new QPushButton(QStringLiteral("Export CSV"), this);
    exportJsonButton_ = new QPushButton(QStringLiteral("Export JSON"), this);
    controls->addWidget(queryButton_);
    controls->addWidget(exportCsvButton_);
    controls->addWidget(exportJsonButton_);
    controls->addStretch(1);
    layout->addLayout(controls);

    auto* chartsLayout = new QHBoxLayout();
    cpuChart_ = ui::createLineChart(cpuSeries_, QStringLiteral("CPU Usage"), 0.0, 100.0,
                                   kMaxChartPoints, QColor("#4f8cff"));
    memoryChart_ = ui::createLineChart(memorySeries_, QStringLiteral("Memory Usage"), 0.0,
                                      100.0, kMaxChartPoints, QColor("#4fd0a0"));
    chartsLayout->addWidget(cpuChart_);
    chartsLayout->addWidget(memoryChart_);
    layout->addLayout(chartsLayout, 1);

    auto* statsLayout = new QHBoxLayout();
    cpuStatsLabel_ = new QLabel(QString(), this);
    memoryStatsLabel_ = new QLabel(QString(), this);
    statsLayout->addWidget(cpuStatsLabel_);
    statsLayout->addWidget(memoryStatsLabel_);
    statsLayout->addStretch(1);
    layout->addLayout(statsLayout);

    table_ = new QTableWidget(this);
    table_->setColumnCount(7);
    table_->setHorizontalHeaderLabels(
        {QStringLiteral("Time"), QStringLiteral("CPU %"), QStringLiteral("Memory"),
         QStringLiteral("Disk read"), QStringLiteral("Disk write"),
         QStringLiteral("Net down"), QStringLiteral("Net up")});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(table_, 1);

    connect(rangeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &AnalyticsPage::onRangeChanged);
    connect(queryButton_, &QPushButton::clicked, this, &AnalyticsPage::onQueryClicked);
    connect(exportCsvButton_, &QPushButton::clicked, this, &AnalyticsPage::onExportCsv);
    connect(exportJsonButton_, &QPushButton::clicked, this, &AnalyticsPage::onExportJson);

    if (service_ != nullptr) {
        connect(service_, &HistoryService::samplesReady, this, &AnalyticsPage::onSamplesReady);
        connect(service_, &HistoryService::statsReady, this, &AnalyticsPage::onStatsReady);
        connect(service_, &HistoryService::exportFinished, this,
                &AnalyticsPage::onExportFinished);
    }
}

TimeRange AnalyticsPage::currentRange() const {
    const qint64 nowMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
    const QString value = rangeCombo_->currentData().toString();
    if (value == QLatin1String("custom")) {
        return {fromEdit_->dateTime().toMSecsSinceEpoch(),
                toEdit_->dateTime().toMSecsSinceEpoch()};
    }
    qint64 spanMs = 3600 * 1000;
    if (value == QLatin1String("6h")) {
        spanMs = 6 * 3600 * 1000;
    } else if (value == QLatin1String("12h")) {
        spanMs = 12 * 3600 * 1000;
    } else if (value == QLatin1String("24h")) {
        spanMs = 24 * 3600 * 1000;
    } else if (value == QLatin1String("7d")) {
        spanMs = 7LL * 24 * 3600 * 1000;
    }
    return {nowMs - spanMs, nowMs};
}

void AnalyticsPage::refresh() {
    onQueryClicked();
}

void AnalyticsPage::onRangeChanged(int index) {
    Q_UNUSED(index);
    const bool custom = rangeCombo_->currentData().toString() == QLatin1String("custom");
    fromEdit_->setVisible(custom);
    toEdit_->setVisible(custom);
}

void AnalyticsPage::onQueryClicked() {
    if (service_ == nullptr) {
        return;
    }
    const TimeRange range = currentRange();
    service_->requestSamples(range);
    service_->requestStats(MetricKind::Cpu, range);
    service_->requestStats(MetricKind::Memory, range);
}

void AnalyticsPage::populateChart() {
    if (cpuSeries_ != nullptr) {
        cpuSeries_->clear();
    }
    if (memorySeries_ != nullptr) {
        memorySeries_->clear();
    }

    const int stride =
        std::max(1, static_cast<int>(samples_.size()) / kMaxChartPoints);
    int pointIndex = 0;
    for (int i = 0; i < samples_.size(); i += stride) {
        const MetricSample& sample = samples_[i];
        if (sample.cpuUsage.has_value() && cpuSeries_ != nullptr) {
            cpuSeries_->append(pointIndex, *sample.cpuUsage);
        }
        const double memory = memoryPercent(sample);
        if ((sample.memoryUsedBytes.has_value() && sample.memoryTotalBytes.has_value()) &&
            memorySeries_ != nullptr) {
            memorySeries_->append(pointIndex, memory);
        }
        ++pointIndex;
    }
}

void AnalyticsPage::populateTable() {
    table_->setRowCount(0);
    const int stride =
        std::max(1, static_cast<int>(samples_.size()) / kMaxTableRows);
    int row = 0;
    for (int i = 0; i < samples_.size() && row < kMaxTableRows; i += stride) {
        const MetricSample& sample = samples_[i];
        table_->insertRow(row);
        table_->setItem(row, 0,
                        new QTableWidgetItem(
                            QDateTime::fromMSecsSinceEpoch(sample.timestampMs)
                                .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))));
        table_->setItem(row, 1,
                        new QTableWidgetItem(sample.cpuUsage.has_value()
                                                 ? QStringLiteral("%1 %")
                                                       .arg(*sample.cpuUsage, 0, 'f', 1)
                                                 : QStringLiteral("—")));
        table_->setItem(
            row, 2,
            new QTableWidgetItem(sample.memoryUsedBytes.has_value()
                                     ? ui::formatBytes(*sample.memoryUsedBytes)
                                     : QStringLiteral("—")));
        table_->setItem(row, 3,
                        new QTableWidgetItem(sample.diskReadRate.has_value()
                                                 ? ui::formatRate(*sample.diskReadRate)
                                                 : QStringLiteral("—")));
        table_->setItem(row, 4,
                        new QTableWidgetItem(sample.diskWriteRate.has_value()
                                                 ? ui::formatRate(*sample.diskWriteRate)
                                                 : QStringLiteral("—")));
        table_->setItem(row, 5,
                        new QTableWidgetItem(sample.netRxRate.has_value()
                                                 ? ui::formatRate(*sample.netRxRate)
                                                 : QStringLiteral("—")));
        table_->setItem(row, 6,
                        new QTableWidgetItem(sample.netTxRate.has_value()
                                                 ? ui::formatRate(*sample.netTxRate)
                                                 : QStringLiteral("—")));
        ++row;
    }
}

void AnalyticsPage::onSamplesReady(const QVector<MetricSample>& samples) {
    samples_ = samples;
    populateChart();
    populateTable();
}

void AnalyticsPage::onStatsReady(MetricKind kind, const MetricStats& stats) {
    if (kind == MetricKind::Cpu) {
        cpuStats_ = stats;
        cpuStatsLabel_->setText(statsText(QStringLiteral("CPU"), stats, QStringLiteral("%")));
    } else if (kind == MetricKind::Memory) {
        memoryStats_ = stats;
        if (stats.count == 0 || !stats.average) {
            memoryStatsLabel_->setText(QStringLiteral("Memory: no data"));
        } else {
            memoryStatsLabel_->setText(
                QStringLiteral("Memory: min %1, max %2, avg %3 (%4 samples)")
                    .arg(ui::formatBytes(static_cast<std::uint64_t>(*stats.minimum)))
                    .arg(ui::formatBytes(static_cast<std::uint64_t>(*stats.maximum)))
                    .arg(ui::formatBytes(static_cast<std::uint64_t>(*stats.average)))
                    .arg(stats.count));
        }
    }
}

void AnalyticsPage::onExportCsv() {
    if (service_ == nullptr) {
        return;
    }
    const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Export CSV"),
                                                     QStringLiteral("metrics.csv"),
                                                     QStringLiteral("CSV files (*.csv)"));
    if (!path.isEmpty()) {
        service_->exportCsv(currentRange(), path);
    }
}

void AnalyticsPage::onExportJson() {
    if (service_ == nullptr) {
        return;
    }
    const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Export JSON"),
                                                     QStringLiteral("metrics.json"),
                                                     QStringLiteral("JSON files (*.json)"));
    if (!path.isEmpty()) {
        service_->exportJson(currentRange(), path);
    }
}

void AnalyticsPage::onExportFinished(bool ok, const QString& path, const QString& error) {
    if (ok) {
        QMessageBox::information(this, QStringLiteral("Export complete"),
                                 QStringLiteral("Saved to:\n%1").arg(path));
    } else {
        QMessageBox::warning(this, QStringLiteral("Export failed"),
                             error.isEmpty() ? QStringLiteral("Unknown error.") : error);
    }
}

} // namespace srm
