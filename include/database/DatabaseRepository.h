#pragma once

#include "database/DatabaseTypes.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct sqlite3;

namespace srm {

class DatabaseRepository {
public:
    DatabaseRepository();
    ~DatabaseRepository();

    DatabaseRepository(const DatabaseRepository&) = delete;
    DatabaseRepository& operator=(const DatabaseRepository&) = delete;

    bool open(const std::string& path);
    void close();
    bool isOpen() const { return db_ != nullptr; }

    bool initializeSchema();

    bool insertSample(const MetricSample& sample);
    std::vector<MetricSample> querySamples(const TimeRange& range);

    MetricStats queryStats(MetricKind kind, const TimeRange& range);

    void setRetentionDays(int days) { retentionDays_ = days > 0 ? days : 1; }
    int retentionDays() const { return retentionDays_; }
    bool prune();
    bool clearHistory();

    bool exportCsv(const TimeRange& range, const std::string& path, std::string& error);
    bool exportJson(const TimeRange& range, const std::string& path, std::string& error);

    std::optional<std::string> getSetting(const std::string& key);
    bool setSetting(const std::string& key, const std::string& value);
    std::vector<AppSetting> allSettings();

    std::string lastError() const { return lastError_; }

private:
    sqlite3* db_{nullptr};
    int retentionDays_{7};
    std::string lastError_;
};

} // namespace srm
