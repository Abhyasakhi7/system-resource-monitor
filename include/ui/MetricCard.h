#pragma once

#include <QWidget>

class QLabel;

namespace srm {

class MetricCard : public QWidget {
    Q_OBJECT

public:
    explicit MetricCard(const QString& title, QWidget* parent = nullptr);

    void setValue(const QString& value);
    void setSubtitle(const QString& subtitle);

private:
    QLabel* valueLabel_;
    QLabel* subtitleLabel_;
};

} // namespace srm
