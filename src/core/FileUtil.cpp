#include "core/FileUtil.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace srm {

std::string trimText(const std::string& text) {
    const auto first = std::find_if_not(text.begin(), text.end(), [](unsigned char character) {
        return std::isspace(character) != 0;
    });
    const auto last = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char character) {
        return std::isspace(character) != 0;
    }).base();
    if (first >= last) {
        return {};
    }
    return std::string(first, last);
}

std::optional<std::string> readFileText(const std::string& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << stream.rdbuf();
    if (stream.bad()) {
        return std::nullopt;
    }
    return buffer.str();
}

std::optional<std::string> readTrimmedFileText(const std::string& path) {
    const auto text = readFileText(path);
    if (!text) {
        return std::nullopt;
    }
    return trimText(*text);
}

} // namespace srm
