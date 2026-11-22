#include <gtest/gtest.h>

#include "database/DatabaseRepository.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace srm;

namespace {

std::int64_t nowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        path_ = std::filesystem::temp_directory_path() / "srm_unit_test.sqlite";
        std::filesystem::remove(path_);
    }

    void TearDown() override { std::filesystem::remove(path_); }

    std::filesystem::path path_;
};

MetricSample makeSample(std::int64_t timestampMs, double cpu) {
    MetricSample sample;
    sample.timestampMs = timestampMs;
    sample.cpuUsage = cpu;
    sample.memoryUsedBytes = 1024ULL * 1024 * 100;
    sample.memoryTotalBytes = 1024ULL * 1024 * 1024;
    return sample;
}

} // namespace

TEST_F(DatabaseTest, InsertsQueriesAndComputesStats) {
    DatabaseRepository repository;
    ASSERT_TRUE(repository.open(path_.string()));
    ASSERT_TRUE(repository.initializeSchema());

    const std::int64_t now = nowMs();
    ASSERT_TRUE(repository.insertSample(makeSample(now - 3 * 3600 * 1000, 10.0)));
    ASSERT_TRUE(repository.insertSample(makeSample(now - 2 * 3600 * 1000, 20.0)));
    ASSERT_TRUE(repository.insertSample(makeSample(now - 1 * 3600 * 1000, 30.0)));

    const TimeRange range{now - 90 * 60 * 1000, now};
    const std::vector<MetricSample> samples = repository.querySamples(range);
    ASSERT_EQ(samples.size(), 1u);
    ASSERT_TRUE(samples.front().cpuUsage.has_value());
    EXPECT_DOUBLE_EQ(*samples.front().cpuUsage, 30.0);

    const MetricStats stats = repository.queryStats(MetricKind::Cpu, range);
    EXPECT_EQ(stats.count, 1u);
    ASSERT_TRUE(stats.minimum.has_value());
    EXPECT_DOUBLE_EQ(*stats.minimum, 30.0);
    ASSERT_TRUE(stats.average.has_value());
    EXPECT_DOUBLE_EQ(*stats.average, 30.0);
}

TEST_F(DatabaseTest, RetentionPruneRemovesOldSamples) {
    DatabaseRepository repository;
    ASSERT_TRUE(repository.open(path_.string()));
    ASSERT_TRUE(repository.initializeSchema());

    const std::int64_t now = nowMs();
    ASSERT_TRUE(repository.insertSample(makeSample(now - 40LL * 24 * 3600 * 1000, 99.0)));
    ASSERT_TRUE(repository.insertSample(makeSample(now - 3600 * 1000, 10.0)));

    repository.setRetentionDays(7);
    ASSERT_TRUE(repository.prune());

    const std::vector<MetricSample> remaining =
        repository.querySamples({now - 50LL * 24 * 3600 * 1000, now});
    ASSERT_EQ(remaining.size(), 1u);
    EXPECT_DOUBLE_EQ(*remaining.front().cpuUsage, 10.0);
}

TEST_F(DatabaseTest, ClearHistoryRemovesAllSamples) {
    DatabaseRepository repository;
    ASSERT_TRUE(repository.open(path_.string()));
    ASSERT_TRUE(repository.initializeSchema());
    ASSERT_TRUE(repository.insertSample(makeSample(nowMs(), 5.0)));
    ASSERT_TRUE(repository.clearHistory());
    EXPECT_TRUE(repository.querySamples({0, nowMs() + 1}).empty());
}

TEST_F(DatabaseTest, SettingsRoundTrip) {
    DatabaseRepository repository;
    ASSERT_TRUE(repository.open(path_.string()));
    ASSERT_TRUE(repository.initializeSchema());

    EXPECT_FALSE(repository.getSetting("interval_ms").has_value());
    ASSERT_TRUE(repository.setSetting("interval_ms", "1500"));
    ASSERT_TRUE(repository.getSetting("interval_ms").has_value());
    EXPECT_EQ(*repository.getSetting("interval_ms"), "1500");

    const std::vector<AppSetting> settings = repository.allSettings();
    ASSERT_EQ(settings.size(), 1u);
    EXPECT_EQ(settings.front().key, "interval_ms");
    EXPECT_EQ(settings.front().value, "1500");
}

TEST_F(DatabaseTest, ExportsCsvAndJson) {
    DatabaseRepository repository;
    ASSERT_TRUE(repository.open(path_.string()));
    ASSERT_TRUE(repository.initializeSchema());
    ASSERT_TRUE(repository.insertSample(makeSample(nowMs(), 12.5)));

    const std::filesystem::path csvPath = path_.string() + ".csv";
    const std::filesystem::path jsonPath = path_.string() + ".json";
    std::string error;

    ASSERT_TRUE(repository.exportCsv({0, nowMs() + 1}, csvPath.string(), error));
    std::ifstream csvStream(csvPath);
    ASSERT_TRUE(csvStream.good());
    std::string line;
    std::getline(csvStream, line);
    EXPECT_NE(line.find("timestamp_ms"), std::string::npos);

    ASSERT_TRUE(repository.exportJson({0, nowMs() + 1}, jsonPath.string(), error));
    std::ifstream jsonStream(jsonPath);
    ASSERT_TRUE(jsonStream.good());
    std::stringstream buffer;
    buffer << jsonStream.rdbuf();
    EXPECT_NE(buffer.str().find("\"cpu_usage\":12.5"), std::string::npos);

    std::filesystem::remove(csvPath);
    std::filesystem::remove(jsonPath);
}
