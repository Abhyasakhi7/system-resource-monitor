#include <gtest/gtest.h>

#include "hardware/HardwareService.h"

using namespace srm;

TEST(HardwareParser, ParsesCpuInfo) {
    const std::string text =
        "processor\t: 0\n"
        "vendor_id\t: GenuineIntel\n"
        "model name\t: Intel(R) Core(TM) i7-9700K CPU @ 3.60GHz\n"
        "cpu MHz\t\t: 3600.000\n"
        "physical id\t: 0\n"
        "core id\t\t: 0\n"
        "\n"
        "processor\t: 1\n"
        "vendor_id\t: GenuineIntel\n"
        "model name\t: Intel(R) Core(TM) i7-9700K CPU @ 3.60GHz\n"
        "cpu MHz\t\t: 3800.000\n"
        "physical id\t: 0\n"
        "core id\t\t: 1\n";

    const CpuInformation info = parseCpuInfo(text);

    EXPECT_TRUE(info.available);
    EXPECT_EQ(info.model, "Intel(R) Core(TM) i7-9700K CPU @ 3.60GHz");
    EXPECT_EQ(info.vendor, "GenuineIntel");
    EXPECT_EQ(info.logicalThreads, 2);
    EXPECT_EQ(info.physicalCores, 2);
    ASSERT_TRUE(info.frequencyMhz.has_value());
    EXPECT_DOUBLE_EQ(*info.frequencyMhz, 3800.0);
}

TEST(HardwareParser, ParsesCpuInfoWithoutModelName) {
    const std::string text =
        "processor\t: 0\n"
        "vendor_id\t: ARM\n"
        "cpu MHz\t\t: 1800.000\n";

    const CpuInformation info = parseCpuInfo(text);
    EXPECT_TRUE(info.available);
    EXPECT_EQ(info.logicalThreads, 1);
    EXPECT_EQ(info.physicalCores, 1);
    EXPECT_EQ(info.vendor, "ARM");
}

TEST(HardwareParser, ParsesOsRelease) {
    const std::string text =
        "NAME=\"Ubuntu\"\n"
        "VERSION=\"24.04.1 LTS (Noble Numbat)\"\n"
        "PRETTY_NAME=\"Ubuntu 24.04.1 LTS\"\n"
        "ID=ubuntu\n";

    const SystemInformation info = parseOsRelease(text);
    EXPECT_TRUE(info.available);
    EXPECT_EQ(info.osPrettyName, "Ubuntu 24.04.1 LTS");
}

TEST(HardwareParser, RejectsMalformedCpuInfo) {
    const CpuInformation info = parseCpuInfo("not a cpuinfo file\n");
    EXPECT_FALSE(info.available);
    EXPECT_EQ(info.logicalThreads, 0);
}
