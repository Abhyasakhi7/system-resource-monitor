#pragma once

#include "process/ProcessInfo.h"
#include "process/ProcessService.h"

#include <QMetaType>
#include <QObject>
#include <QTimer>
#include <QVector>

namespace srm {

class ProcessWorker : public QObject {
    Q_OBJECT

public:
    explicit ProcessWorker(QObject* parent = nullptr);
    ~ProcessWorker() override;

    int intervalMs() const { return intervalMs_; }

public slots:
    void start();
    void stop();
    void setInterval(int intervalMs);

signals:
    void processesReady(const QVector<srm::ProcessInfo>& processes);
    void stopped();

private slots:
    void onTimeout();

private:
    ProcessService service_;
    QTimer* timer_;
    int intervalMs_{2000};
};

} // namespace srm

Q_DECLARE_METATYPE(srm::ProcessInfo)
Q_DECLARE_METATYPE(QVector<srm::ProcessInfo>)
