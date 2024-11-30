#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPixmap>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QTimer>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <QString>

class SystemInfo : public QWidget
{
    Q_OBJECT

public:
    explicit SystemInfo(QWidget *parent = nullptr);
    ~SystemInfo()
    {
        delete hardwareLabel;
        delete systemStatusLabel;
        delete hostnameLabel;
        delete distributionLabel;
        delete osInfoLabel;
        delete kernelInfoLabel;
        delete desktopLabel;
        delete memoryLabel;
        delete cpuLabel;
        delete diskLabel;
        // delete refreshTimer;
    }

private slots:
    void refreshInfo(); // Function to update system information

private:
    QString getHostname();
    QString getDistributionName();
    QString getOSReleaseVersion();
    QString getKernelVersion();
    QString getMemoryInfo();
    QString getProcessorVersion();
    QString getDiskStorageInfo();
    QString getDesktopEnvironment();
    QPixmap getDesktopIcon();

    QLabel *hardwareLabel;
    QLabel *systemStatusLabel;
    QLabel *hostnameLabel;
    QLabel *distributionLabel;
    QLabel *osInfoLabel;
    QLabel *kernelInfoLabel;
    QLabel *desktopLabel;
    QLabel *memoryLabel;
    QLabel *cpuLabel;
    QLabel *diskLabel;

    // QTimer *refreshTimer;
};

#endif // SYSTEMINFO_H
