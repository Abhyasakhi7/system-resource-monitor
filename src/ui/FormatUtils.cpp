#include "ui/FormatUtils.h"

#include <QString>

#include <cmath>

namespace srm::ui {

QString formatBytes(std::uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double value = static_cast<double>(bytes);
    int unit = 0;
    while (value >= 1024.0 && unit < 4) {
        value /= 1024.0;
        ++unit;
    }
    return QString::number(value, 'f', unit == 0 ? 0 : 1) + QStringLiteral(" ") +
           QString::fromLatin1(units[unit]);
}

QString formatRate(double bytesPerSec, bool bits) {
    if (!std::isfinite(bytesPerSec)) {
        return QStringLiteral("—");
    }

    double value = bytesPerSec;
    if (bits) {
        value *= 8.0;
    }

    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    while (value >= 1024.0 && unit < 4) {
        value /= 1024.0;
        ++unit;
    }

    QString text = QString::number(value, 'f', unit == 0 ? 0 : 1) + QStringLiteral(" ") +
                   QString::fromLatin1(units[unit]);
    if (bits) {
        text += QStringLiteral("it/s");
    } else {
        text += QStringLiteral("/s");
    }
    return text;
}

QString formatPercent(double value) {
    return QString::number(value, 'f', 1) + QStringLiteral(" %");
}

} // namespace srm::ui
