#include "ui/ProcessPage.h"
#include "ui/FormatUtils.h"

#include "process/ProcessService.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <csignal>

namespace srm {

namespace {

QString signalResultText(ProcessSignalResult result) {
    switch (result) {
        case ProcessSignalResult::Ok:
            return QStringLiteral("Signal sent.");
        case ProcessSignalResult::NoSuchProcess:
            return QStringLiteral("Process no longer exists (or the PID was reused).");
        case ProcessSignalResult::PermissionDenied:
            return QStringLiteral("Permission denied. The process was not signalled.");
        case ProcessSignalResult::InvalidPid:
            return QStringLiteral("Invalid PID.");
        case ProcessSignalResult::Failed:
            return QStringLiteral("Failed to send the signal.");
    }
    return QStringLiteral("Unknown result.");
}

bool processMatches(const ProcessInfo& process, const QString& filter) {
    if (filter.isEmpty()) {
        return true;
    }
    const QString text = filter.toLower();
    return QString::fromStdString(process.name).toLower().contains(text) ||
           QString::fromStdString(process.user).toLower().contains(text) ||
           QString::number(process.pid).contains(text);
}

} // namespace

ProcessPage::ProcessPage(ProcessService* service, QWidget* parent)
    : QWidget(parent), service_(service) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto* controls = new QHBoxLayout();
    search_ = new QLineEdit(this);
    search_->setPlaceholderText(QStringLiteral("Search by name, user, or PID"));
    search_->setClearButtonEnabled(true);
    controls->addWidget(search_, 1);

    terminateButton_ = new QPushButton(QStringLiteral("Terminate (SIGTERM)"), this);
    killButton_ = new QPushButton(QStringLiteral("Kill (SIGKILL)"), this);
    terminateButton_->setEnabled(false);
    killButton_->setEnabled(false);
    controls->addWidget(terminateButton_);
    controls->addWidget(killButton_);
    layout->addLayout(controls);

    table_ = new QTableWidget(this);
    table_->setColumnCount(6);
    table_->setHorizontalHeaderLabels({QStringLiteral("PID"), QStringLiteral("Name"),
                                       QStringLiteral("User"), QStringLiteral("State"),
                                       QStringLiteral("CPU %"), QStringLiteral("Memory")});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSortingEnabled(false);
    layout->addWidget(table_, 2);

    detail_ = new QTreeWidget(this);
    detail_->setHeaderHidden(true);
    detail_->setColumnCount(2);
    layout->addWidget(detail_, 1);

    statusLabel_ = new QLabel(QString(), this);
    statusLabel_->setStyleSheet(QStringLiteral("color:#9aa0a6;"));
    layout->addWidget(statusLabel_);

    connect(search_, &QLineEdit::textChanged, this, &ProcessPage::onSearchChanged);
    connect(table_->horizontalHeader(), &QHeaderView::sectionClicked, this,
            &ProcessPage::onSortByColumn);
    connect(table_, &QTableWidget::itemSelectionChanged, this,
            &ProcessPage::onSelectionChanged);
    connect(terminateButton_, &QPushButton::clicked, this, &ProcessPage::onTerminate);
    connect(killButton_, &QPushButton::clicked, this, &ProcessPage::onKill);
}

void ProcessPage::updateProcesses(const QVector<ProcessInfo>& processes) {
    processes_ = processes;
    rebuildTable();
}

void ProcessPage::rebuildTable() {
    const QString filter = search_ != nullptr ? search_->text() : QString();

    QVector<ProcessInfo> visible;
    for (const ProcessInfo& process : processes_) {
        if (processMatches(process, filter)) {
            visible.append(process);
        }
    }

    std::sort(visible.begin(), visible.end(), [this](const ProcessInfo& a, const ProcessInfo& b) {
        switch (sortColumn_) {
            case 0:
                return sortOrder_ == Qt::AscendingOrder ? a.pid < b.pid : a.pid > b.pid;
            case 1:
                return sortOrder_ == Qt::AscendingOrder
                           ? a.name < b.name
                           : a.name > b.name;
            case 2:
                return sortOrder_ == Qt::AscendingOrder
                           ? a.user < b.user
                           : a.user > b.user;
            case 3:
                return sortOrder_ == Qt::AscendingOrder
                           ? a.state < b.state
                           : a.state > b.state;
            case 4: {
                const double left = a.cpuPercent.value_or(-1.0);
                const double right = b.cpuPercent.value_or(-1.0);
                return sortOrder_ == Qt::AscendingOrder ? left < right : left > right;
            }
            case 5: {
                const std::uint64_t left = a.rssBytes.value_or(0);
                const std::uint64_t right = b.rssBytes.value_or(0);
                return sortOrder_ == Qt::AscendingOrder ? left < right : left > right;
            }
            default:
                return a.pid < b.pid;
        }
    });

    table_->setRowCount(0);
    for (const ProcessInfo& process : visible) {
        const int row = table_->rowCount();
        table_->insertRow(row);

        auto* pidItem = new QTableWidgetItem(QString::number(process.pid));
        pidItem->setData(Qt::UserRole, process.pid);
        table_->setItem(row, 0, pidItem);
        table_->setItem(row, 1,
                        new QTableWidgetItem(QString::fromStdString(process.name)));
        table_->setItem(row, 2,
                        new QTableWidgetItem(QString::fromStdString(process.user)));
        table_->setItem(row, 3,
                        new QTableWidgetItem(QString::fromStdString(process.state)));
        table_->setItem(row, 4,
                        new QTableWidgetItem(process.cpuPercent.has_value()
                                                 ? QStringLiteral("%1 %")
                                                       .arg(*process.cpuPercent, 0, 'f', 1)
                                                 : QStringLiteral("—")));
        table_->setItem(row, 5,
                        new QTableWidgetItem(process.rssBytes.has_value()
                                                 ? ui::formatBytes(*process.rssBytes)
                                                 : QStringLiteral("—")));
    }

    if (currentPid_ > 0) {
        bool found = false;
        for (int row = 0; row < table_->rowCount(); ++row) {
            QTableWidgetItem* item = table_->item(row, 0);
            if (item != nullptr && item->data(Qt::UserRole).toInt() == currentPid_) {
                table_->setCurrentItem(item);
                found = true;
                break;
            }
        }
        if (!found) {
            currentPid_ = -1;
            currentStartTicks_ = 0;
            detail_->clear();
            terminateButton_->setEnabled(false);
            killButton_->setEnabled(false);
        }
    }
}

void ProcessPage::showDetail(const ProcessInfo& process) {
    detail_->clear();

    const auto addItem = [this](const QString& label, const QString& value) {
        auto* item = new QTreeWidgetItem(detail_);
        item->setText(0, label);
        item->setText(1, value);
    };

    addItem(QStringLiteral("PID"), QString::number(process.pid));
    addItem(QStringLiteral("Name"), QString::fromStdString(process.name));
    addItem(QStringLiteral("User"), QString::fromStdString(process.user));
    addItem(QStringLiteral("State"), QString::fromStdString(process.state));
    addItem(QStringLiteral("CPU %"), process.cpuPercent.has_value()
                                    ? QStringLiteral("%1 %").arg(*process.cpuPercent, 0, 'f', 1)
                                    : QStringLiteral("—"));
    addItem(QStringLiteral("Memory (RSS)"),
            process.rssBytes.has_value() ? ui::formatBytes(*process.rssBytes)
                                         : QStringLiteral("—"));
    addItem(QStringLiteral("Virtual memory"),
            process.vmSizeBytes.has_value() ? ui::formatBytes(*process.vmSizeBytes)
                                            : QStringLiteral("—"));
    addItem(QStringLiteral("Parent PID"), QString::number(process.parentPid));
    addItem(QStringLiteral("Threads"), QString::number(process.threadCount));
    addItem(QStringLiteral("Start ticks"), QString::number(process.startTicks));
}

void ProcessPage::onSelectionChanged() {
    const QList<QTableWidgetItem*> selected = table_->selectedItems();
    if (selected.isEmpty()) {
        currentPid_ = -1;
        currentStartTicks_ = 0;
        detail_->clear();
        terminateButton_->setEnabled(false);
        killButton_->setEnabled(false);
        return;
    }

    QTableWidgetItem* pidItem = table_->item(selected.first()->row(), 0);
    if (pidItem == nullptr) {
        return;
    }

    const int pid = pidItem->data(Qt::UserRole).toInt();
    for (const ProcessInfo& process : processes_) {
        if (process.pid == pid) {
            currentPid_ = pid;
            currentStartTicks_ = process.startTicks;
            showDetail(process);
            terminateButton_->setEnabled(true);
            killButton_->setEnabled(true);
            break;
        }
    }
}

void ProcessPage::onSearchChanged(const QString& text) {
    Q_UNUSED(text);
    rebuildTable();
}

void ProcessPage::onSortByColumn(int column) {
    if (sortColumn_ == column) {
        sortOrder_ = sortOrder_ == Qt::AscendingOrder ? Qt::DescendingOrder
                                                      : Qt::AscendingOrder;
    } else {
        sortColumn_ = column;
        sortOrder_ = Qt::AscendingOrder;
    }
    rebuildTable();
}

QString ProcessPage::resultMessage(ProcessSignalResult result) const {
    return signalResultText(result);
}

void ProcessPage::onTerminate() {
    if (service_ == nullptr || currentPid_ <= 0) {
        return;
    }

    const QString name = detail_->topLevelItemCount() > 1
                             ? detail_->topLevelItem(1)->text(1)
                             : QString::number(currentPid_);
    const auto answer = QMessageBox::question(
        this, QStringLiteral("Terminate process"),
        QStringLiteral("Send SIGTERM to process %1 (%2)?").arg(currentPid_).arg(name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }

    const ProcessSignalResult result =
        service_->sendSignal(currentPid_, SIGTERM, currentStartTicks_);
    statusLabel_->setText(resultMessage(result));
}

void ProcessPage::onKill() {
    if (service_ == nullptr || currentPid_ <= 0) {
        return;
    }

    const QString name = detail_->topLevelItemCount() > 1
                             ? detail_->topLevelItem(1)->text(1)
                             : QString::number(currentPid_);
    const auto answer = QMessageBox::warning(
        this, QStringLiteral("Force kill process"),
        QStringLiteral("Send SIGKILL to process %1 (%2)? Unsaved data may be lost.")
            .arg(currentPid_)
            .arg(name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }

    const ProcessSignalResult result =
        service_->sendSignal(currentPid_, SIGKILL, currentStartTicks_);
    statusLabel_->setText(resultMessage(result));
}

} // namespace srm
