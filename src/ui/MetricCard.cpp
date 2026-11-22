#include "ui/MetricCard.h"

#include <QLabel>
#include <QVBoxLayout>

namespace srm {

MetricCard::MetricCard(const QString& title, QWidget* parent)
    : QWidget(parent) {
    setStyleSheet(QStringLiteral("background-color:#2a2b30; border-radius:10px;"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(6);

    auto* titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet(QStringLiteral("color:#9aa0a6; font-size:13px;"));
    layout->addWidget(titleLabel);

    valueLabel_ = new QLabel(QStringLiteral("—"), this);
    valueLabel_->setStyleSheet(
        QStringLiteral("color:#f5f5f5; font-size:24px; font-weight:600;"));
    layout->addWidget(valueLabel_);

    subtitleLabel_ = new QLabel(QString(), this);
    subtitleLabel_->setStyleSheet(QStringLiteral("color:#8a8f98; font-size:12px;"));
    layout->addWidget(subtitleLabel_);
}

void MetricCard::setValue(const QString& value) {
    valueLabel_->setText(value);
}

void MetricCard::setSubtitle(const QString& subtitle) {
    subtitleLabel_->setText(subtitle);
}

} // namespace srm
