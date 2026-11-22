#pragma once

#include "process/ProcessInfo.h"

#include <QVector>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTreeWidget;
QT_END_NAMESPACE

namespace srm {

class ProcessService;

class ProcessPage : public QWidget {
    Q_OBJECT

public:
    explicit ProcessPage(ProcessService* service, QWidget* parent = nullptr);

public slots:
    void updateProcesses(const QVector<ProcessInfo>& processes);

private slots:
    void onTerminate();
    void onKill();
    void onSearchChanged(const QString& text);
    void onSortByColumn(int column);
    void onSelectionChanged();

private:
    void rebuildTable();
    void showDetail(const ProcessInfo& process);
    QString resultMessage(ProcessSignalResult result) const;

    ProcessService* service_{nullptr};
    QVector<ProcessInfo> processes_;
    QTableWidget* table_{nullptr};
    QLineEdit* search_{nullptr};
    QTreeWidget* detail_{nullptr};
    QLabel* statusLabel_{nullptr};
    QPushButton* terminateButton_{nullptr};
    QPushButton* killButton_{nullptr};
    int currentPid_{-1};
    std::uint64_t currentStartTicks_{0};
    int sortColumn_{0};
    Qt::SortOrder sortOrder_{Qt::AscendingOrder};
};

} // namespace srm
