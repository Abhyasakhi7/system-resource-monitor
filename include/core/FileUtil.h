#pragma once

#include <optional>
#include <string>

namespace srm {

std::string trimText(const std::string& text);
std::optional<std::string> readFileText(const std::string& path);
std::optional<std::string> readTrimmedFileText(const std::string& path);

} // namespace srm
