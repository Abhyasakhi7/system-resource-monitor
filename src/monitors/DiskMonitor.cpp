#include "monitors/DiskMonitor.h"

#include "core/FileUtil.h"
#include "core/MetricMath.h"

#include <QStorageInfo>

#include <regex>
#include <sstream>
#include <utility>

namespace srm {

namespace {

constexpr std::uint64_t kSectorSize = 512;

bool isPhysicalDisk(const std::string& name) {
    static const std::regex pattern(
        R"(^(sd[a-z]+|hd[a-z]+|vd[a-z]+|xvd[a-z]+|nvme\d+n\d+|mmcblk\d+)$)");
    return std::regex_match(name, pattern);
}

} // namespace

DiskRawSample parseProcDiskstats(const std::string& text) {
    DiskRawSample sample;
    std::istringstream stream(text);
    std::string line;

    while (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        int major = 0;
        int minor = 0;
        std::string name;
        std::uint64_t readsCompleted = 0;
        std::uint64_t readsMerged = 0;
        std::uint64_t sectorsRead = 0;
        std::uint64_t msReading = 0;
        std::uint64_t writesCompleted = 0;
        std::uint64_t writesMerged = 0;
        std::uint64_t sectorsWritten = 0;

        if (!(lineStream >> major >> minor >> name >> readsCompleted >> readsMerged >>
              sectorsRead >> msReading >> writesCompleted >> writesMerged >> sectorsWritten)) {
            continue;
        }

        if (!isPhysicalDisk(name)) {
            continue;
        }

        sample.sectorsRead += sectorsRead;
        sample.sectorsWritten += sectorsWritten;
        sample.valid = true;
    }

    return sample;
}

DiskMonitor::DiskMonitor(std::string diskstatsPath)
    : diskstatsPath_(std::move(diskstatsPath)) {}

bool DiskMonitor::refresh() {
    const auto text = readFileText(diskstatsPath_);
    if (!text) {
        available_ = false;
        metrics_ = DiskMetrics{};
        return false;
    }

    const DiskRawSample current = parseProcDiskstats(*text);
    if (!current.valid) {
        available_ = false;
        metrics_ = DiskMetrics{};
        return false;
    }

    const TimePoint now = Clock::now();
    if (previous_.has_value()) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - previousTime_);
        metrics_.readBytesPerSec =
            computeByteRate(previous_->sectorsRead * kSectorSize,
                            current.sectorsRead * kSectorSize, elapsed);
        metrics_.writeBytesPerSec =
            computeByteRate(previous_->sectorsWritten * kSectorSize,
                            current.sectorsWritten * kSectorSize, elapsed);
    } else {
        metrics_.readBytesPerSec.reset();
        metrics_.writeBytesPerSec.reset();
    }

    previous_ = current;
    previousTime_ = now;

    refreshFilesystems();

    available_ = true;
    return true;
}

void DiskMonitor::refreshFilesystems() {
    metrics_.filesystems.clear();

    const QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo& volume : volumes) {
        if (!volume.isValid() ||
            !volume.device().startsWith(QByteArrayLiteral("/dev/"))) {
            continue;
        }

        FilesystemMetrics filesystem;
        filesystem.mountPoint = volume.rootPath().toStdString();
        if (volume.bytesTotal() > 0) {
            filesystem.totalBytes = static_cast<std::uint64_t>(volume.bytesTotal());
        }
        if (volume.bytesFree() >= 0) {
            filesystem.freeBytes = static_cast<std::uint64_t>(volume.bytesFree());
        }
        metrics_.filesystems.push_back(std::move(filesystem));
    }
}

} // namespace srm
