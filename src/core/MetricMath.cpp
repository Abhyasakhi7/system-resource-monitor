#include "core/MetricMath.h"

namespace srm {

std::optional<double> computeByteRate(std::uint64_t previousBytes,
                                      std::uint64_t currentBytes,
                                      std::chrono::milliseconds elapsed) {
    if (elapsed.count() <= 0) {
        return std::nullopt;
    }
    if (currentBytes < previousBytes) {
        return std::nullopt;
    }

    const double seconds = static_cast<double>(elapsed.count()) / 1000.0;
    return static_cast<double>(currentBytes - previousBytes) / seconds;
}

} // namespace srm
