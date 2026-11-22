#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

namespace srm {

std::optional<double> computeByteRate(std::uint64_t previousBytes,
                                      std::uint64_t currentBytes,
                                      std::chrono::milliseconds elapsed);

} // namespace srm
