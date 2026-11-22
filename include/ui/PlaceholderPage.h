#pragma once

#include <QWidget>

class QLabel;

namespace srm {

class PlaceholderPage : public QWidget {
    Q_OBJECT

public:
    PlaceholderPage(const QString& title, const QString& message, QWidget* parent = nullptr);

private:
    QLabel* messageLabel_{nullptr};
};

} // namespace srm
