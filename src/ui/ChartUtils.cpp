#include "ui/ChartUtils.h"

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <QList>
#include <QBrush>
#include <QPainter>

namespace srm::ui {

QChartView* createLineChart(QLineSeries*& series, const QString& title, double minimumY,
                            double maximumY, int maxSamples, const QColor& color) {
    auto* chart = new QChart();
    chart->setTitle(title);
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setTitleBrush(QBrush(QColor("#e6e6e6")));

    series = new QLineSeries();
    series->setColor(color);
    chart->addSeries(series);
    chart->createDefaultAxes();

    const QList<QAbstractAxis*> horizontalAxes = chart->axes(Qt::Horizontal);
    const QList<QAbstractAxis*> verticalAxes = chart->axes(Qt::Vertical);
    if (!horizontalAxes.isEmpty()) {
        auto* axisX = qobject_cast<QValueAxis*>(horizontalAxes.first());
        if (axisX != nullptr) {
            axisX->setRange(0, maxSamples - 1);
            axisX->setTickCount(7);
            axisX->setLabelsColor(QColor("#9aa0a6"));
        }
    }
    if (!verticalAxes.isEmpty()) {
        auto* axisY = qobject_cast<QValueAxis*>(verticalAxes.first());
        if (axisY != nullptr) {
            axisY->setRange(minimumY, maximumY);
            axisY->setTickCount(6);
            axisY->setLabelsColor(QColor("#9aa0a6"));
        }
    }

    auto* view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumHeight(220);
    return view;
}

} // namespace srm::ui
