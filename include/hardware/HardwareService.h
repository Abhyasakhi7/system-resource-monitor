#pragma once

#include "hardware/HardwareInfo.h"

#include <string>

namespace srm {

CpuInformation parseCpuInfo(const std::string& text);
SystemInformation parseOsRelease(const std::string& text);

class HardwareService {
public:
    HardwareService() = default;

    HardwareReport collect();
};

} // namespace srm
