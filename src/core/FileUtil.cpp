#include "core/FileUtil.h"

#include <fstream>
#include <sstream>

namespace srm {

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

} // namespace srm
