#include "database/DatabaseRepository.h"

#include <sqlite3.h>

#include <chrono>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace srm {

namespace {

class Statement {
public:
    Statement(sqlite3* db, const char* sql)
        : statement_(nullptr) {
        sqlite3_prepare_v2(db, sql, -1, &statement_, nullptr);
    }

    ~Statement() {
        if (statement_ != nullptr) {
            sqlite3_finalize(statement_);
        }
    }

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    sqlite3_stmt* get() const { return statement_; }
    bool valid() const { return statement_ != nullptr; }

private:
    sqlite3_stmt* statement_;
};

const char* metricColumn(MetricKind kind) {
    switch (kind) {
        case MetricKind::Cpu:
            return "cpu_usage";
        case MetricKind::Memory:
            return "memory_used";
        case MetricKind::DiskRead:
            return "disk_read_rate";
        case MetricKind::DiskWrite:
            return "disk_write_rate";
        case MetricKind::NetworkRx:
            return "net_rx_rate";
        case MetricKind::NetworkTx:
            return "net_tx_rate";
    }
    return "cpu_usage";
}

void bindSample(sqlite3_stmt* statement, const MetricSample& sample) {
    sqlite3_bind_int64(statement, 1, sample.timestampMs);
    if (sample.cpuUsage) {
        sqlite3_bind_double(statement, 2, *sample.cpuUsage);
    } else {
        sqlite3_bind_null(statement, 2);
    }
    if (sample.memoryUsedBytes) {
        sqlite3_bind_int64(statement, 3, static_cast<sqlite3_int64>(*sample.memoryUsedBytes));
    } else {
        sqlite3_bind_null(statement, 3);
    }
    if (sample.memoryTotalBytes) {
        sqlite3_bind_int64(statement, 4, static_cast<sqlite3_int64>(*sample.memoryTotalBytes));
    } else {
        sqlite3_bind_null(statement, 4);
    }
    if (sample.diskReadRate) {
        sqlite3_bind_double(statement, 5, *sample.diskReadRate);
    } else {
        sqlite3_bind_null(statement, 5);
    }
    if (sample.diskWriteRate) {
        sqlite3_bind_double(statement, 6, *sample.diskWriteRate);
    } else {
        sqlite3_bind_null(statement, 6);
    }
    if (sample.netRxRate) {
        sqlite3_bind_double(statement, 7, *sample.netRxRate);
    } else {
        sqlite3_bind_null(statement, 7);
    }
    if (sample.netTxRate) {
        sqlite3_bind_double(statement, 8, *sample.netTxRate);
    } else {
        sqlite3_bind_null(statement, 8);
    }
}

MetricSample readSample(sqlite3_stmt* statement) {
    MetricSample sample;
    sample.timestampMs = sqlite3_column_int64(statement, 0);
    if (sqlite3_column_type(statement, 1) != SQLITE_NULL) {
        sample.cpuUsage = sqlite3_column_double(statement, 1);
    }
    if (sqlite3_column_type(statement, 2) != SQLITE_NULL) {
        sample.memoryUsedBytes =
            static_cast<std::uint64_t>(sqlite3_column_int64(statement, 2));
    }
    if (sqlite3_column_type(statement, 3) != SQLITE_NULL) {
        sample.memoryTotalBytes =
            static_cast<std::uint64_t>(sqlite3_column_int64(statement, 3));
    }
    if (sqlite3_column_type(statement, 4) != SQLITE_NULL) {
        sample.diskReadRate = sqlite3_column_double(statement, 4);
    }
    if (sqlite3_column_type(statement, 5) != SQLITE_NULL) {
        sample.diskWriteRate = sqlite3_column_double(statement, 5);
    }
    if (sqlite3_column_type(statement, 6) != SQLITE_NULL) {
        sample.netRxRate = sqlite3_column_double(statement, 6);
    }
    if (sqlite3_column_type(statement, 7) != SQLITE_NULL) {
        sample.netTxRate = sqlite3_column_double(statement, 7);
    }
    return sample;
}

std::string optionalToString(const std::optional<double>& value) {
    if (!value) {
        return std::string();
    }
    std::ostringstream stream;
    stream.precision(6);
    stream << std::fixed << *value;
    return stream.str();
}

std::string optionalBytesToString(const std::optional<std::uint64_t>& value) {
    return value ? std::to_string(*value) : std::string();
}

} // namespace

DatabaseRepository::DatabaseRepository() = default;

DatabaseRepository::~DatabaseRepository() {
    close();
}

bool DatabaseRepository::open(const std::string& path) {
    close();

    if (sqlite3_open_v2(path.c_str(), &db_,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                        nullptr) != SQLITE_OK) {
        lastError_ = db_ != nullptr ? sqlite3_errmsg(db_) : "unable to open database";
        close();
        return false;
    }

    sqlite3_busy_timeout(db_, 5000);
    return true;
}

void DatabaseRepository::close() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DatabaseRepository::initializeSchema() {
    if (db_ == nullptr) {
        lastError_ = "database not open";
        return false;
    }

    const char* schema =
        "CREATE TABLE IF NOT EXISTS metric_samples ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp INTEGER NOT NULL,"
        "cpu_usage REAL,"
        "memory_used INTEGER,"
        "memory_total INTEGER,"
        "disk_read_rate REAL,"
        "disk_write_rate REAL,"
        "net_rx_rate REAL,"
        "net_tx_rate REAL);"
        "CREATE INDEX IF NOT EXISTS idx_metric_samples_timestamp "
        "ON metric_samples(timestamp);"
        "CREATE TABLE IF NOT EXISTS app_settings ("
        "key TEXT PRIMARY KEY,"
        "value TEXT NOT NULL,"
        "updated_at INTEGER NOT NULL);";

    char* errorMessage = nullptr;
    if (sqlite3_exec(db_, schema, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        lastError_ = errorMessage != nullptr ? errorMessage : "schema creation failed";
        sqlite3_free(errorMessage);
        return false;
    }
    return true;
}

bool DatabaseRepository::insertSample(const MetricSample& sample) {
    if (db_ == nullptr) {
        lastError_ = "database not open";
        return false;
    }

    const char* sql =
        "INSERT INTO metric_samples (timestamp, cpu_usage, memory_used, memory_total, "
        "disk_read_rate, disk_write_rate, net_rx_rate, net_tx_rate) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    Statement statement(db_, sql);
    if (!statement.valid()) {
        lastError_ = sqlite3_errmsg(db_);
        return false;
    }

    bindSample(statement.get(), sample);
    if (sqlite3_step(statement.get()) != SQLITE_DONE) {
        lastError_ = sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

std::vector<MetricSample> DatabaseRepository::querySamples(const TimeRange& range) {
    std::vector<MetricSample> samples;
    if (db_ == nullptr || !range.valid()) {
        return samples;
    }

    const char* sql =
        "SELECT timestamp, cpu_usage, memory_used, memory_total, disk_read_rate, "
        "disk_write_rate, net_rx_rate, net_tx_rate FROM metric_samples "
        "WHERE timestamp >= ? AND timestamp <= ? ORDER BY timestamp ASC;";
    Statement statement(db_, sql);
    if (!statement.valid()) {
        lastError_ = sqlite3_errmsg(db_);
        return samples;
    }

    sqlite3_bind_int64(statement.get(), 1, range.fromMs);
    sqlite3_bind_int64(statement.get(), 2, range.toMs);

    while (sqlite3_step(statement.get()) == SQLITE_ROW) {
        samples.push_back(readSample(statement.get()));
    }
    return samples;
}

MetricStats DatabaseRepository::queryStats(MetricKind kind, const TimeRange& range) {
    MetricStats stats;
    if (db_ == nullptr || !range.valid()) {
        return stats;
    }

    std::ostringstream sql;
    sql << "SELECT COUNT(" << metricColumn(kind) << "), MIN(" << metricColumn(kind)
        << "), MAX(" << metricColumn(kind) << "), AVG(" << metricColumn(kind)
        << ") FROM metric_samples WHERE timestamp >= ? AND timestamp <= ?;";

    Statement statement(db_, sql.str().c_str());
    if (!statement.valid()) {
        lastError_ = sqlite3_errmsg(db_);
        return stats;
    }

    sqlite3_bind_int64(statement.get(), 1, range.fromMs);
    sqlite3_bind_int64(statement.get(), 2, range.toMs);

    if (sqlite3_step(statement.get()) == SQLITE_ROW) {
        stats.count = static_cast<std::uint64_t>(sqlite3_column_int64(statement.get(), 0));
        if (sqlite3_column_type(statement.get(), 1) != SQLITE_NULL) {
            stats.minimum = sqlite3_column_double(statement.get(), 1);
        }
        if (sqlite3_column_type(statement.get(), 2) != SQLITE_NULL) {
            stats.maximum = sqlite3_column_double(statement.get(), 2);
        }
        if (sqlite3_column_type(statement.get(), 3) != SQLITE_NULL) {
            stats.average = sqlite3_column_double(statement.get(), 3);
        }
    }
    return stats;
}

bool DatabaseRepository::prune() {
    if (db_ == nullptr) {
        lastError_ = "database not open";
        return false;
    }

    const std::int64_t cutoffMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
                                      .count() -
                                  static_cast<std::int64_t>(retentionDays_) * 24 * 60 * 60 * 1000;

    Statement statement(db_, "DELETE FROM metric_samples WHERE timestamp < ?;");
    if (!statement.valid()) {
        lastError_ = sqlite3_errmsg(db_);
        return false;
    }
    sqlite3_bind_int64(statement.get(), 1, cutoffMs);
    if (sqlite3_step(statement.get()) != SQLITE_DONE) {
        lastError_ = sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

bool DatabaseRepository::clearHistory() {
    if (db_ == nullptr) {
        lastError_ = "database not open";
        return false;
    }
    char* errorMessage = nullptr;
    if (sqlite3_exec(db_, "DELETE FROM metric_samples;", nullptr, nullptr, &errorMessage) !=
        SQLITE_OK) {
        lastError_ = errorMessage != nullptr ? errorMessage : "clear history failed";
        sqlite3_free(errorMessage);
        return false;
    }
    return true;
}

bool DatabaseRepository::exportCsv(const TimeRange& range, const std::string& path,
                                   std::string& error) {
    const std::vector<MetricSample> samples = querySamples(range);
    if (!range.valid()) {
        error = "invalid time range";
        return false;
    }

    std::ofstream stream(path);
    if (!stream) {
        error = "unable to open output file";
        return false;
    }

    stream << "timestamp_ms,cpu_usage,memory_used,memory_total,disk_read_rate,"
              "disk_write_rate,net_rx_rate,net_tx_rate\n";
    for (const MetricSample& sample : samples) {
        stream << sample.timestampMs << ',' << optionalToString(sample.cpuUsage) << ','
               << optionalBytesToString(sample.memoryUsedBytes) << ','
               << optionalBytesToString(sample.memoryTotalBytes) << ','
               << optionalToString(sample.diskReadRate) << ','
               << optionalToString(sample.diskWriteRate) << ','
               << optionalToString(sample.netRxRate) << ','
               << optionalToString(sample.netTxRate) << '\n';
    }
    return stream.good();
}

bool DatabaseRepository::exportJson(const TimeRange& range, const std::string& path,
                                    std::string& error) {
    if (!range.valid()) {
        error = "invalid time range";
        return false;
    }

    const std::vector<MetricSample> samples = querySamples(range);

    std::ofstream stream(path);
    if (!stream) {
        error = "unable to open output file";
        return false;
    }

    stream << "[\n";
    for (std::size_t i = 0; i < samples.size(); ++i) {
        const MetricSample& sample = samples[i];
        stream << "  {\"timestamp_ms\":" << sample.timestampMs
               << ",\"cpu_usage\":"
               << (sample.cpuUsage ? optionalToString(sample.cpuUsage) : std::string("null"))
               << ",\"memory_used\":"
               << (sample.memoryUsedBytes ? std::to_string(*sample.memoryUsedBytes)
                                          : std::string("null"))
               << ",\"memory_total\":"
               << (sample.memoryTotalBytes ? std::to_string(*sample.memoryTotalBytes)
                                           : std::string("null"))
               << ",\"disk_read_rate\":"
               << (sample.diskReadRate ? optionalToString(sample.diskReadRate)
                                       : std::string("null"))
               << ",\"disk_write_rate\":"
               << (sample.diskWriteRate ? optionalToString(sample.diskWriteRate)
                                        : std::string("null"))
               << ",\"net_rx_rate\":"
               << (sample.netRxRate ? optionalToString(sample.netRxRate) : std::string("null"))
               << ",\"net_tx_rate\":"
               << (sample.netTxRate ? optionalToString(sample.netTxRate) : std::string("null"))
               << '}';
        if (i + 1 < samples.size()) {
            stream << ',';
        }
        stream << '\n';
    }
    stream << "]\n";
    return stream.good();
}

std::optional<std::string> DatabaseRepository::getSetting(const std::string& key) {
    if (db_ == nullptr) {
        return std::nullopt;
    }

    Statement statement(db_, "SELECT value FROM app_settings WHERE key = ?;");
    if (!statement.valid()) {
        lastError_ = sqlite3_errmsg(db_);
        return std::nullopt;
    }
    sqlite3_bind_text(statement.get(), 1, key.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(statement.get()) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(statement.get(), 0);
        if (text != nullptr) {
            return std::string(reinterpret_cast<const char*>(text));
        }
    }
    return std::nullopt;
}

bool DatabaseRepository::setSetting(const std::string& key, const std::string& value) {
    if (db_ == nullptr) {
        lastError_ = "database not open";
        return false;
    }

    const std::int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    Statement statement(db_,
                        "INSERT INTO app_settings (key, value, updated_at) VALUES (?, ?, ?) "
                        "ON CONFLICT(key) DO UPDATE SET value = excluded.value, "
                        "updated_at = excluded.updated_at;");
    if (!statement.valid()) {
        lastError_ = sqlite3_errmsg(db_);
        return false;
    }
    sqlite3_bind_text(statement.get(), 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement.get(), 2, value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(statement.get(), 3, nowMs);

    if (sqlite3_step(statement.get()) != SQLITE_DONE) {
        lastError_ = sqlite3_errmsg(db_);
        return false;
    }
    return true;
}

std::vector<AppSetting> DatabaseRepository::allSettings() {
    std::vector<AppSetting> settings;
    if (db_ == nullptr) {
        return settings;
    }

    Statement statement(db_, "SELECT key, value FROM app_settings;");
    if (!statement.valid()) {
        lastError_ = sqlite3_errmsg(db_);
        return settings;
    }

    while (sqlite3_step(statement.get()) == SQLITE_ROW) {
        const unsigned char* key = sqlite3_column_text(statement.get(), 0);
        const unsigned char* value = sqlite3_column_text(statement.get(), 1);
        if (key != nullptr && value != nullptr) {
            settings.push_back({reinterpret_cast<const char*>(key),
                                reinterpret_cast<const char*>(value)});
        }
    }
    return settings;
}

} // namespace srm
