#include "ui/PlaceholderPage.h"

#include <QLabel>
#include <QVBoxLayout>

namespace srm {

PlaceholderPage::PlaceholderPage(const QString& title, const QString& message,
                                 QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);

    auto* titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet(QStringLiteral("font-size:20px; font-weight:600;"));
    layout->addWidget(titleLabel);

    messageLabel_ = new QLabel(message, this);
    messageLabel_->setWordWrap(true);
    messageLabel_->setStyleSheet(QStringLiteral("color:#9aa0a6;"));
    layout->addWidget(messageLabel_);

    layout->addStretch(1);
}

} // namespace srm
