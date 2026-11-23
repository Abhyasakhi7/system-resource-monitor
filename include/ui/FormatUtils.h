#pragma once

#include <cstdint>

class QString;

namespace srm::ui {

QString formatBytes(std::uint64_t bytes);
QString formatRate(double bytesPerSec, bool bits = false);
QString formatPercent(double value);
QString formatCelsius(double celsius);

} // namespace srm::ui
