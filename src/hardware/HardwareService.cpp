#include "hardware/HardwareService.h"

#include "core/FileUtil.h"

#include <QDir>
#include <QStorageInfo>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>
#include <sys/utsname.h>
#include <unistd.h>
#include <unordered_set>

namespace srm {

namespace {

std::string trim(const std::string& text) {
    const auto first = std::find_if_not(text.begin(), text.end(), [](unsigned char c) {
        return std::isspace(c) != 0;
    });
    const auto last = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char c) {
        return std::isspace(c) != 0;
    }).base();
    if (first >= last) {
        return {};
    }
    return std::string(first, last);
}

std::optional<std::string> readSmallFile(const std::string& path) {
    const auto text = readFileText(path);
    if (!text) {
        return std::nullopt;
    }
    return trim(*text);
}

std::string readValue(const std::string& text, const std::string& key) {
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        const auto colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        if (trim(line.substr(0, colon)) == key) {
            return trim(line.substr(colon + 1));
        }
    }
    return {};
}

std::string readAssignedValue(const std::string& text, const std::string& key) {
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        const auto equals = line.find('=');
        if (equals == std::string::npos) {
            continue;
        }
        if (trim(line.substr(0, equals)) == key) {
            return trim(line.substr(equals + 1));
        }
    }
    return {};
}

std::vector<std::string> listDirectories(const std::string& path) {
    std::vector<std::string> entries;
    QDir directory(QString::fromStdString(path));
    if (!directory.exists()) {
        return entries;
    }
    const QStringList names = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& name : names) {
        entries.push_back(name.toStdString());
    }
    return entries;
}

std::string sysfsPath(const std::string& card, const std::string& file) {
    return "/sys/class/drm/" + card + "/device/" + file;
}

std::string stripPartitionSuffix(const std::string& device) {
    std::string name = device;
    const std::size_t slash = name.find_last_of('/');
    if (slash != std::string::npos) {
        name = name.substr(slash + 1);
    }

    const auto stripTrailingNumber = [](std::string& value) {
        std::size_t end = value.size();
        while (end > 0 && std::isdigit(static_cast<unsigned char>(value[end - 1])) != 0) {
            --end;
        }
        value.resize(end);
    };

    if (name.rfind("nvme", 0) == 0 || name.rfind("mmcblk", 0) == 0) {
        const std::size_t partition = name.rfind('p');
        if (partition != std::string::npos && partition + 1 < name.size()) {
            bool allDigits = true;
            for (std::size_t i = partition + 1; i < name.size(); ++i) {
                if (std::isdigit(static_cast<unsigned char>(name[i])) == 0) {
                    allDigits = false;
                    break;
                }
            }
            if (allDigits) {
                name.resize(partition);
            }
        }
        return name;
    }

    stripTrailingNumber(name);
    return name;
}

std::string blockDeviceModel(const std::string& device) {
    const std::string base = stripPartitionSuffix(device);
    const auto model = readSmallFile("/sys/block/" + base + "/device/model");
    const auto vendor = readSmallFile("/sys/block/" + base + "/device/vendor");
    if (model && vendor) {
        return *vendor + " " + *model;
    }
    if (model) {
        return *model;
    }
    if (vendor) {
        return *vendor;
    }
    return {};
}

} // namespace

CpuInformation parseCpuInfo(const std::string& text) {
    CpuInformation info;
    info.source = "/proc/cpuinfo";

    std::istringstream stream(text);
    std::string line;
    std::set<std::pair<std::string, std::string>> coreIds;
    std::string currentPhysicalId;
    int processorCount = 0;
    double maxFrequency = 0.0;

    while (std::getline(stream, line)) {
        const auto colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        const std::string key = trim(line.substr(0, colon));
        const std::string value = trim(line.substr(colon + 1));

        if (key == "processor") {
            ++processorCount;
        } else if (key == "model name" && info.model.empty()) {
            info.model = value;
        } else if (key == "vendor_id" && info.vendor.empty()) {
            info.vendor = value;
        } else if (key == "cpu MHz") {
            const double frequency = std::stod(value);
            maxFrequency = std::max(maxFrequency, frequency);
        } else if (key == "physical id") {
            currentPhysicalId = value;
        } else if (key == "core id") {
            coreIds.insert({currentPhysicalId, value});
        }
    }

    info.logicalThreads = processorCount;
    info.physicalCores = static_cast<int>(coreIds.size());
    if (info.physicalCores == 0) {
        info.physicalCores = processorCount;
    }
    if (maxFrequency > 0.0) {
        info.frequencyMhz = maxFrequency;
    }
    info.available = !info.model.empty() || processorCount > 0;
    return info;
}

SystemInformation parseOsRelease(const std::string& text) {
    SystemInformation info;
    info.source = "/etc/os-release";

    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        const auto equals = line.find('=');
        if (equals == std::string::npos) {
            continue;
        }
        const std::string key = trim(line.substr(0, equals));
        std::string value = trim(line.substr(equals + 1));
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }

        if (key == "PRETTY_NAME") {
            info.osPrettyName = value;
        } else if (key == "NAME" && info.osPrettyName.empty()) {
            info.osPrettyName = value;
        }
    }

    info.available = !info.osPrettyName.empty();
    return info;
}

HardwareReport HardwareService::collect() {
    HardwareReport report;

    if (const auto cpuinfo = readFileText("/proc/cpuinfo")) {
        report.cpu = parseCpuInfo(*cpuinfo);
    }

    utsname unameData{};
    if (uname(&unameData) == 0) {
        report.cpu.architecture = unameData.machine;
        report.system.kernelVersion = unameData.release;
        report.system.architecture = unameData.machine;
    }

    if (const auto meminfo = readFileText("/proc/meminfo")) {
        const std::string memTotal = readValue(*meminfo, "MemTotal");
        if (!memTotal.empty()) {
            std::istringstream stream(memTotal);
            std::uint64_t kb = 0;
            if (stream >> kb) {
                report.memory.totalBytes = kb * 1024ULL;
                report.memory.available = true;
                report.memory.source = "/proc/meminfo";
            }
        }
    }
    report.memory.modules.clear();

    SystemInformation osInfo;
    if (const auto osRelease = readFileText("/etc/os-release")) {
        osInfo = parseOsRelease(*osRelease);
    }
    char hostname[256] = {0};
    if (gethostname(hostname, sizeof(hostname) - 1) == 0) {
        osInfo.hostname = hostname;
    }
    if (uname(&unameData) == 0) {
        osInfo.kernelVersion = unameData.release;
        osInfo.architecture = unameData.machine;
    }
    if (osInfo.source.empty()) {
        osInfo.source = "/etc/os-release, uname";
    }
    osInfo.available = !osInfo.osPrettyName.empty() || !osInfo.hostname.empty() ||
                       !osInfo.kernelVersion.empty();
    report.system = osInfo;

    for (const std::string& card : listDirectories("/sys/class/drm")) {
        if (card.find("card") != 0) {
            continue;
        }
        const auto vendor = readSmallFile(sysfsPath(card, "vendor"));
        const auto device = readSmallFile(sysfsPath(card, "device"));
        if (!vendor && !device) {
            continue;
        }

        GpuInformation gpu;
        gpu.vendorId = vendor.value_or("");
        gpu.deviceId = device.value_or("");
        gpu.name = gpu.vendorId;
        if (!gpu.deviceId.empty()) {
            gpu.name += (gpu.name.empty() ? "" : " ") + gpu.deviceId;
        }
        const auto driver = readSmallFile(sysfsPath(card, "uevent"));
        if (driver) {
            const std::string driverValue = readAssignedValue(*driver, "DRIVER");
            if (!driverValue.empty()) {
                gpu.driver = driverValue;
            }
        }
        const auto vram = readSmallFile(sysfsPath(card, "mem_info_vram_total"));
        if (vram) {
            std::istringstream stream(*vram);
            std::uint64_t bytes = 0;
            if (stream >> bytes) {
                gpu.vramBytes = bytes;
            }
        }
        gpu.available = true;
        gpu.source = "/sys/class/drm/" + card + "/device";
        report.gpu = gpu;
        break;
    }

    const QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo& volume : volumes) {
        if (!volume.isValid() || !volume.device().startsWith(QByteArrayLiteral("/dev/"))) {
            continue;
        }

        StorageDeviceInformation device;
        device.device = volume.device().toStdString();
        device.mountPoint = volume.rootPath().toStdString();
        device.filesystem = volume.fileSystemType().toStdString();
        if (volume.bytesTotal() > 0) {
            device.totalBytes = static_cast<std::uint64_t>(volume.bytesTotal());
        }
        const std::string model = blockDeviceModel(device.device);
        if (!model.empty()) {
            device.model = model;
        }
        report.storage.devices.push_back(std::move(device));
    }
    if (!report.storage.devices.empty()) {
        report.storage.available = true;
        report.storage.source = "/proc/mounts, QStorageInfo";
    }

    const auto boardVendor = readSmallFile("/sys/class/dmi/id/board_vendor");
    const auto boardName = readSmallFile("/sys/class/dmi/id/board_name");
    const auto sysVendor = readSmallFile("/sys/class/dmi/id/sys_vendor");
    const auto productName = readSmallFile("/sys/class/dmi/id/product_name");
    if (boardVendor || boardName) {
        report.motherboard.manufacturer = sysVendor.value_or(boardVendor.value_or(""));
        report.motherboard.model = boardName.value_or("");
        report.motherboard.productName = productName.value_or("");
        report.motherboard.available = true;
        report.motherboard.source = "/sys/class/dmi/id";
    }

    for (const std::string& supply : listDirectories("/sys/class/power_supply")) {
        if (supply.find("BAT") != 0) {
            continue;
        }
        const std::string basePath = "/sys/class/power_supply/" + supply;
        const auto capacity = readSmallFile(basePath + "/capacity");
        const auto status = readSmallFile(basePath + "/status");
        const auto health = readSmallFile(basePath + "/health");
        if (!capacity && !status) {
            continue;
        }

        BatteryInformation battery;
        battery.name = supply;
        if (capacity) {
            battery.capacityPercent = std::stoi(*capacity);
        }
        battery.status = status.value_or("");
        battery.health = health.value_or("");
        battery.available = true;
        battery.source = basePath;
        report.battery = battery;
        break;
    }

    return report;
}

} // namespace srm
