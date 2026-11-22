#include "process/ProcessWorker.h"

#include <QTimer>
#include <QVector>

#include <algorithm>

namespace srm {

ProcessWorker::ProcessWorker(QObject* parent)
    : QObject(parent), timer_(new QTimer(this)) {
    timer_->setInterval(intervalMs_);
    connect(timer_, &QTimer::timeout, this, &ProcessWorker::onTimeout);
}

ProcessWorker::~ProcessWorker() = default;

void ProcessWorker::start() {
    timer_->start(intervalMs_);
}

void ProcessWorker::stop() {
    timer_->stop();
    emit stopped();
}

void ProcessWorker::setInterval(int intervalMs) {
    intervalMs_ = std::max(500, intervalMs);
    if (timer_->isActive()) {
        timer_->stop();
        timer_->start(intervalMs_);
    } else {
        timer_->setInterval(intervalMs_);
    }
}

void ProcessWorker::onTimeout() {
    const std::vector<ProcessInfo> processes = service_.refresh();
    QVector<ProcessInfo> result;
    result.reserve(static_cast<int>(processes.size()));
    for (const ProcessInfo& process : processes) {
        result.append(process);
    }
    emit processesReady(result);
}

} // namespace srm
