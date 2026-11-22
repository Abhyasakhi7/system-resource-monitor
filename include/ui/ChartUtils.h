#pragma once

#include <QColor>
#include <QString>

QT_BEGIN_NAMESPACE
class QChartView;
class QLineSeries;
QT_END_NAMESPACE

namespace srm::ui {

QChartView* createLineChart(QLineSeries*& series, const QString& title, double minimumY,
                            double maximumY, int maxSamples, const QColor& color);

} // namespace srm::ui
