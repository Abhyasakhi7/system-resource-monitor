#include <gtest/gtest.h>

#include "monitors/BatteryMonitor.h"
#include "monitors/TemperatureMonitor.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

using namespace srm;

namespace {

std::filesystem::path makeTempDir(const std::string& name) {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() /
        ("srm_" + name + "_" + std::to_string(stamp));
    std::filesystem::create_directories(path);
    return path;
}

void writeFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream stream(path, std::ios::binary);
    stream << content;
}

} // namespace

TEST(SensorParser, ParsesTemperatureMillidegrees) {
    EXPECT_EQ(parseTemperatureMilliC(" 52000\n"), 52000);
    EXPECT_EQ(parseTemperatureMilliC("-1250"), -1250);
    EXPECT_EQ(parseTemperatureMilliC("0\n"), 0);
}

TEST(SensorParser, RejectsInvalidTemperature) {
    EXPECT_FALSE(parseTemperatureMilliC("").has_value());
    EXPECT_FALSE(parseTemperatureMilliC("unavailable").has_value());
    EXPECT_FALSE(parseTemperatureMilliC("52 C").has_value());
    EXPECT_FALSE(parseTemperatureMilliC("500000").has_value());
}

TEST(SensorParser, ParsesBatteryCapacity) {
    EXPECT_EQ(parseBatteryCapacity("85\n"), 85);
    EXPECT_EQ(parseBatteryCapacity("0"), 0);
    EXPECT_EQ(parseBatteryCapacity("100"), 100);
    EXPECT_FALSE(parseBatteryCapacity("101").has_value());
    EXPECT_FALSE(parseBatteryCapacity("-1").has_value());
    EXPECT_FALSE(parseBatteryCapacity("unknown").has_value());
}

TEST(SensorParser, CalculatesChargePercent) {
    EXPECT_EQ(calculateChargePercent("5000000", "10000000"), 50);
    EXPECT_FALSE(calculateChargePercent("1000", "0").has_value());
    EXPECT_FALSE(calculateChargePercent("30000000", "10000000").has_value());
}

TEST(TemperatureMonitor, DiscoversThermalZonesAndHwmonSensors) {
    const std::filesystem::path root = makeTempDir("temperature");
    const std::filesystem::path thermal = root / "thermal";
    const std::filesystem::path hwmon = root / "hwmon";
    std::filesystem::create_directories(thermal / "thermal_zone0");
    std::filesystem::create_directories(thermal / "thermal_zone1");
    std::filesystem::create_directories(hwmon / "hwmon0");

    writeFile(thermal / "thermal_zone0" / "type", "x86_pkg_temp\n");
    writeFile(thermal / "thermal_zone0" / "temp", "54000\n");
    writeFile(thermal / "thermal_zone1" / "type", "acpitz\n");
    writeFile(thermal / "thermal_zone1" / "temp", "invalid\n");
    writeFile(hwmon / "hwmon0" / "name", "coretemp\n");
    writeFile(hwmon / "hwmon0" / "temp1_input", "62000\n");
    writeFile(hwmon / "hwmon0" / "temp1_label", "Package 0\n");
    writeFile(hwmon / "hwmon0" / "temp2_input", "58000\n");

    TemperatureMonitor monitor(thermal.string(), hwmon.string());
    EXPECT_TRUE(monitor.refresh());

    const TemperatureMetrics& metrics = monitor.metrics();
    ASSERT_EQ(metrics.sensors.size(), 3U);
    EXPECT_EQ(metrics.sensors[0].name, "coretemp Package 0");
    ASSERT_TRUE(metrics.sensors[0].celsius.has_value());
    EXPECT_DOUBLE_EQ(*metrics.sensors[0].celsius, 62.0);
    EXPECT_EQ(metrics.sensors[1].name, "coretemp temp2");
    EXPECT_EQ(metrics.sensors[2].name, "x86_pkg_temp");
    EXPECT_DOUBLE_EQ(*metrics.sensors[2].celsius, 54.0);

    std::filesystem::remove_all(root);
}

TEST(TemperatureMonitor, ReportsUnavailableWithoutSensors) {
    const std::filesystem::path root = makeTempDir("temperature_empty");
    TemperatureMonitor monitor((root / "thermal").string(), (root / "hwmon").string());
    EXPECT_FALSE(monitor.refresh());
    EXPECT_TRUE(monitor.metrics().sensors.empty());
    std::filesystem::remove_all(root);
}

TEST(BatteryMonitor, ReadsCapacityStatusAndHealth) {
    const std::filesystem::path root = makeTempDir("battery");
    const std::filesystem::path battery = root / "BAT0";
    std::filesystem::create_directories(battery);
    writeFile(battery / "name", "BAT0\n");
    writeFile(battery / "capacity", "72\n");
    writeFile(battery / "status", "Discharging\n");
    writeFile(battery / "health", "Good\n");

    BatteryMonitor monitor(root.string());
    EXPECT_TRUE(monitor.refresh());
    EXPECT_EQ(monitor.metrics().name, "BAT0");
    ASSERT_TRUE(monitor.metrics().capacityPercent.has_value());
    EXPECT_EQ(*monitor.metrics().capacityPercent, 72);
    EXPECT_EQ(monitor.metrics().status, "Discharging");
    EXPECT_EQ(monitor.metrics().health, "Good");

    std::filesystem::remove_all(root);
}

TEST(BatteryMonitor, FallsBackToEnergyCounters) {
    const std::filesystem::path root = makeTempDir("battery_energy");
    const std::filesystem::path battery = root / "BAT1";
    std::filesystem::create_directories(battery);
    writeFile(battery / "status", "Charging\n");
    writeFile(battery / "energy_now", "25000000\n");
    writeFile(battery / "energy_full", "50000000\n");

    BatteryMonitor monitor(root.string());
    EXPECT_TRUE(monitor.refresh());
    EXPECT_EQ(monitor.metrics().name, "BAT1");
    ASSERT_TRUE(monitor.metrics().capacityPercent.has_value());
    EXPECT_EQ(*monitor.metrics().capacityPercent, 50);

    std::filesystem::remove_all(root);
}

TEST(BatteryMonitor, ReportsUnavailableWithoutBattery) {
    const std::filesystem::path root = makeTempDir("battery_empty");
    BatteryMonitor monitor(root.string());
    EXPECT_FALSE(monitor.refresh());
    std::filesystem::remove_all(root);
}
