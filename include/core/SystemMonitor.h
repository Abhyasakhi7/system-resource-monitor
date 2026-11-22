#pragma once

#include "core/DataCollector.h"

#include <QMetaType>
#include <QObject>
#include <QTimer>

namespace srm {

class SystemMonitor : public QObject {
    Q_OBJECT

public:
    explicit SystemMonitor(QObject* parent = nullptr);
    ~SystemMonitor() override;

    int intervalMs() const { return intervalMs_; }

public slots:
    void start();
    void stop();
    void setInterval(int intervalMs);
    void setCollectorEnabled(bool cpu, bool memory, bool disk, bool network);

signals:
    void snapshotReady(const srm::SystemMetricsSnapshot& snapshot);
    void stopped();

private slots:
    void onTimeout();

private:
    DataCollector collector_;
    QTimer* timer_;
    int intervalMs_{1000};
};

} // namespace srm

Q_DECLARE_METATYPE(srm::SystemMetricsSnapshot)
