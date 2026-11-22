#pragma once

#include <cstdint>

class QString;

namespace srm::ui {

QString formatBytes(std::uint64_t bytes);
QString formatRate(double bytesPerSec, bool bits = false);
QString formatPercent(double value);

} // namespace srm::ui
