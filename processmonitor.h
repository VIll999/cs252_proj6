#ifndef PROCESS_MONITOR_H
#define PROCESS_MONITOR_H

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QProcess>
#include <QDialog>
#include <QDebug>
#include <QString>
#include <signal.h>
#include <unistd.h>

class ProcessMonitor : public QWidget
{
    Q_OBJECT

public:
    ProcessMonitor(QWidget *parent = nullptr);

private:
    QLabel *loadLabel;
    QTreeWidget *processTree;
    QPushButton *endProcessButton;

    QString getUsername(uid_t uid);

    void refreshProcesses();

    void showContextMenu(const QPoint &pos);

    void updateButtonState();

    void endProcess();

    int getSelectedPID();

    void stopProcess();

    void continueProcess();

    void killProcess();

    void showMemoryMaps();

    void showOpenFiles();

    void showProcessDetails();

    void showOpenFilesDialog(QString processName, int pid);

    void showMemoryMapsDialog(QString processName, int pid);
};

#endif // PROCESS_MONITOR_H
