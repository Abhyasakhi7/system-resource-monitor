#pragma once

#include <optional>
#include <string>

namespace srm {

std::optional<std::string> readFileText(const std::string& path);

} // namespace srm
