#include <gtest/gtest.h>

#include "core/MetricMath.h"
#include "monitors/CpuMonitor.h"
#include "monitors/DiskMonitor.h"
#include "monitors/MemoryMonitor.h"
#include "monitors/NetworkMonitor.h"

#include <chrono>

using namespace srm;

namespace {

std::chrono::milliseconds ms(long long value) {
    return std::chrono::milliseconds(value);
}

} // namespace

TEST(CpuParser, ParsesAggregateAndPerCore) {
    const std::string text =
        "cpu  100 20 30 400 10 5 5 0 0 0\n"
        "cpu0 50 10 15 200 5 2 2 0 0 0\n"
        "cpu1 50 10 15 200 5 3 3 0 0 0\n"
        "intr 123 0\n";

    const CpuRawSample sample = parseProcStat(text);

    ASSERT_TRUE(sample.valid);
    EXPECT_EQ(sample.totalTicks, 570u);
    EXPECT_EQ(sample.idleTicks, 410u);
    ASSERT_EQ(sample.cores.size(), 2u);
    EXPECT_EQ(sample.cores[0].first, 284u);
    EXPECT_EQ(sample.cores[0].second, 205u);
    EXPECT_EQ(sample.cores[1].first, 286u);
    EXPECT_EQ(sample.cores[1].second, 205u);
}

TEST(CpuParser, IgnoresMalformedInput) {
    EXPECT_FALSE(parseProcStat("").valid);
    EXPECT_FALSE(parseProcStat("hello world\n").valid);
    EXPECT_FALSE(parseProcStat("cpu\n").valid);
}

TEST(CpuUsage, ComputesDelta) {
    CpuRawSample previous;
    previous.valid = true;
    previous.totalTicks = 1000;
    previous.idleTicks = 900;

    CpuRawSample current;
    current.valid = true;
    current.totalTicks = 1100;
    current.idleTicks = 950;

    const auto usage = computeCpuUsagePercent(previous, current, ms(1000));
    ASSERT_TRUE(usage.has_value());
    EXPECT_DOUBLE_EQ(*usage, 50.0);
}

TEST(CpuUsage, RejectsInvalidInput) {
    CpuRawSample previous;
    previous.valid = true;
    previous.totalTicks = 1000;
    previous.idleTicks = 900;

    CpuRawSample current = previous;

    EXPECT_FALSE(computeCpuUsagePercent(previous, current, ms(0)).has_value());

    CpuRawSample reset = previous;
    reset.totalTicks = 500;
    EXPECT_FALSE(computeCpuUsagePercent(previous, reset, ms(1000)).has_value());

    CpuRawSample negativeIdle = previous;
    negativeIdle.idleTicks = 800;
    EXPECT_FALSE(computeCpuUsagePercent(previous, negativeIdle, ms(1000)).has_value());

    CpuRawSample same = previous;
    EXPECT_FALSE(computeCpuUsagePercent(previous, same, ms(1000)).has_value());
}

TEST(CpuUsage, ComputesPerCoreDelta) {
    CpuRawSample previous;
    previous.valid = true;
    previous.cores = {{1000, 900}, {2000, 1900}};

    CpuRawSample current;
    current.valid = true;
    current.cores = {{1100, 950}, {2200, 1950}};

    const std::vector<double> usage = computePerCoreUsagePercent(previous, current);
    ASSERT_EQ(usage.size(), 2u);
    EXPECT_DOUBLE_EQ(usage[0], 50.0);
    EXPECT_DOUBLE_EQ(usage[1], 75.0);
}

TEST(CpuParser, ParsesFrequency) {
    EXPECT_DOUBLE_EQ(parseFrequencyKhz("2400000\n").value(), 2400000.0);
    EXPECT_FALSE(parseFrequencyKhz("not-a-number\n").has_value());
    EXPECT_FALSE(parseFrequencyKhz("-10\n").has_value());
}

TEST(MemoryParser, ParsesKeys) {
    const std::string text =
        "MemTotal:       16000000 kB\n"
        "MemFree:         4000000 kB\n"
        "MemAvailable:    8000000 kB\n"
        "Buffers:          200000 kB\n"
        "Cached:          1500000 kB\n"
        "SwapTotal:       2000000 kB\n"
        "SwapFree:        1000000 kB\n";

    const MemoryRawSample sample = parseProcMeminfo(text);

    ASSERT_TRUE(sample.valid);
    ASSERT_TRUE(sample.memTotalKb.has_value());
    EXPECT_EQ(*sample.memTotalKb, std::uint64_t(16000000));
    EXPECT_EQ(*sample.memAvailableKb, std::uint64_t(8000000));
    EXPECT_EQ(*sample.cachedKb, std::uint64_t(1500000));
    EXPECT_EQ(*sample.swapTotalKb, std::uint64_t(2000000));
    EXPECT_EQ(*sample.swapFreeKb, std::uint64_t(1000000));
}

TEST(MemoryParser, MissingMemAvailableIsStillValid) {
    const MemoryRawSample sample = parseProcMeminfo("MemTotal: 8000000 kB\n");
    EXPECT_TRUE(sample.valid);
    EXPECT_FALSE(sample.memAvailableKb.has_value());
}

TEST(DiskParser, AggregatesPhysicalDisksOnly) {
    const std::string text =
        "8 0 sda 100 200 300 400 500 0 600\n"
        "8 1 sda1 100 200 900 400 500 0 900\n"
        "7 0 loop0 100 200 900 400 500 0 900\n"
        "259 0 nvme0n1 100 200 300 400 500 0 600\n"
        "259 1 nvme0n1p1 100 200 900 400 500 0 900\n";

    const DiskRawSample sample = parseProcDiskstats(text);

    ASSERT_TRUE(sample.valid);
    EXPECT_EQ(sample.sectorsRead, 600u);
    EXPECT_EQ(sample.sectorsWritten, 1200u);
}

TEST(DiskParser, IgnoresMalformedLines) {
    EXPECT_FALSE(parseProcDiskstats("").valid);
    EXPECT_FALSE(parseProcDiskstats("8 0 sda 1 2 3\n").valid);
}

TEST(MetricMath, ComputesByteRate) {
    EXPECT_DOUBLE_EQ(computeByteRate(512, 1536, ms(1000)).value(), 1024.0);
    EXPECT_DOUBLE_EQ(computeByteRate(0, 0, ms(1000)).value(), 0.0);
    EXPECT_FALSE(computeByteRate(1536, 512, ms(1000)).has_value());
    EXPECT_FALSE(computeByteRate(512, 1536, ms(0)).has_value());
}

TEST(NetworkParser, ParsesInterfaces) {
    const std::string text =
        "Inter-|   Receive                                                |  Transmit\n"
        " face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n"
        "    lo: 100 1 0 0 0 0 0 0 200 2 0 0 0 0 0 0\n"
        "  eth0: 1000 10 0 0 0 0 0 0 2000 20 0 0 0 0 0 0\n";

    const NetworkRawSample sample = parseProcNetDev(text);

    ASSERT_TRUE(sample.valid);
    ASSERT_EQ(sample.interfaces.size(), 2u);
    EXPECT_EQ(sample.interfaces[0].name, "lo");
    EXPECT_EQ(sample.interfaces[0].rxBytes, 100u);
    EXPECT_EQ(sample.interfaces[0].txBytes, 200u);
    EXPECT_EQ(sample.interfaces[1].name, "eth0");
    EXPECT_EQ(sample.interfaces[1].rxBytes, 1000u);
    EXPECT_EQ(sample.interfaces[1].txBytes, 2000u);
}

TEST(NetworkParser, IgnoresMalformedLines) {
    EXPECT_FALSE(parseProcNetDev("").valid);
    EXPECT_FALSE(parseProcNetDev("eth0: 1 2 3\n").valid);
}
