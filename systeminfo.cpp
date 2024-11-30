#include "systeminfo.h"
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QDebug>
#include <QImage>
#include <unistd.h>
#include <QDebug>

SystemInfo::SystemInfo(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QWidget *hostnameWidget = new QWidget(this);
    QHBoxLayout *hostnameLayout = new QHBoxLayout(hostnameWidget);

    QLabel *iconLabel = new QLabel(this);
    // iconLabel->setPixmap(getDesktopIcon());
    // iconLabel->setFixedSize(48, 48);

    // Hostname
    hostnameLabel = new QLabel(getHostname(), this);
    QFont titleFont = hostnameLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    hostnameLabel->setFont(titleFont);

    hostnameLayout->addWidget(iconLabel);
    hostnameLayout->addWidget(hostnameLabel);
    hostnameLayout->addStretch(); // keep the hostname label to the left
    mainLayout->addWidget(hostnameWidget);

    // System Information
    QGroupBox *systemGroup = new QGroupBox(this);
    QVBoxLayout *systemLayout = new QVBoxLayout(systemGroup);

    distributionLabel = new QLabel(getDistributionName(), this);
    hardwareLabel = new QLabel("Hardware", this);
    systemStatusLabel = new QLabel("System Status", this);
    osInfoLabel = new QLabel("\t" + getOSReleaseVersion(), this);
    kernelInfoLabel = new QLabel("\t" + getKernelVersion(), this);
    desktopLabel = new QLabel("\t" + getDesktopEnvironment(), this);
    memoryLabel = new QLabel("\tMemory: " + getMemoryInfo(), this);
    cpuLabel = new QLabel("\tProcessor: " + getProcessorVersion(), this);
    diskLabel = new QLabel("\tAvailable disk space: " + getDiskStorageInfo(), this);

    QFont infoFont;
    infoFont.setPointSize(14);

    osInfoLabel->setFont(infoFont);
    kernelInfoLabel->setFont(infoFont);
    desktopLabel->setFont(infoFont);
    memoryLabel->setFont(infoFont);
    cpuLabel->setFont(infoFont);
    diskLabel->setFont(infoFont);
    infoFont.setBold(true);
    distributionLabel->setFont(infoFont);
    hardwareLabel->setFont(infoFont);
    systemStatusLabel->setFont(infoFont);

    // Add widgets to layout
    systemLayout->addWidget(distributionLabel);
    systemLayout->addWidget(osInfoLabel);
    systemLayout->addWidget(kernelInfoLabel);
    systemLayout->addWidget(desktopLabel);
    systemLayout->addWidget(hardwareLabel);
    systemLayout->addWidget(memoryLabel);
    systemLayout->addWidget(cpuLabel);
    systemLayout->addWidget(systemStatusLabel);
    systemLayout->addWidget(diskLabel);

    systemGroup->setLayout(systemLayout);
    mainLayout->addWidget(systemGroup);

    setLayout(mainLayout);

    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &SystemInfo::refreshInfo);
    refreshTimer->start(10000);
}

void SystemInfo::refreshInfo()
{
    hostnameLabel->setText(getHostname());
    distributionLabel->setText(getDistributionName());
    osInfoLabel->setText("\t" + getOSReleaseVersion());
    kernelInfoLabel->setText("\t" + getKernelVersion());
    desktopLabel->setText("\t" + getDesktopEnvironment());
    memoryLabel->setText("\tMemory: " + getMemoryInfo());
    cpuLabel->setText("\tProcessor: " + getProcessorVersion());
    diskLabel->setText("\tAvailable disk space: " + getDiskStorageInfo());

    qDebug() << "System information refreshed";
}

QString SystemInfo::getHostname()
{
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        return "Unknown Hostname";
    }
    return QString(hostname);
}

QString SystemInfo::getDistributionName()
{
    QFile file("/etc/os-release");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return "Unknown Distribution";
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.startsWith("NAME="))
        {
            file.close();
            return line.split("=").last().replace("\"", "");
        }
    }
    file.close();
    return "Unknown Distribution";
}

QString SystemInfo::getOSReleaseVersion()
{
    QFile file("/etc/os-release");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "OS Version: Unknown";

    QTextStream in(&file);
    QString line;
    while (!in.atEnd())
    {
        line = in.readLine();
        if (line.startsWith("PRETTY_NAME="))
        {
            file.close();
            return line.mid(line.indexOf("=") + 1).remove("\"").trimmed();
        }
    }
    file.close();
    return "OS Version: Unknown";
}

QString SystemInfo::getKernelVersion()
{
    struct utsname buffer;
    if (uname(&buffer) == 0)
    {
        return QString("Kernel %1 %2\.%3").arg(buffer.sysname).arg(buffer.release).arg(buffer.machine);
    }

    return "Kernel: N/A";
}

QString SystemInfo::getProcessorVersion()
{
    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return "N/A";
    }

    QByteArray fileContent = file.readAll();
    file.close();

    QList<QByteArray> lines = fileContent.split('\n');
    for (const QByteArray &line : lines)
    {
        if (line.startsWith("model name"))
        {
            QList<QByteArray> parts = line.split(':');
            if (parts.size() > 1)
            {
                return QString(parts[1].trimmed());
            }
        }
    }

    return "Unknown CPU";
}

QString SystemInfo::getMemoryInfo()
{
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "N/A";

    QTextStream in(&file);
    QString memInfo = in.readLine();
    file.close();

    QString memValue = memInfo.split(":").last().trimmed().split(" ").first();
    double memInGB = memValue.toDouble() / 1024 / 1024;
    return QString("%1 GB").arg(memInGB, 0, 'f', 2);
}

QString SystemInfo::getDiskStorageInfo()
{
    struct statvfs stat;
    if (statvfs("/", &stat) != 0)
    {
        return "N/A";
    }

    unsigned long long total = stat.f_blocks * stat.f_frsize;
    unsigned long long free = stat.f_bfree * stat.f_frsize;
    double freeInGB = free / (1024 * 1024 * 1024);
    return QString("%1 GB").arg(freeInGB, 0, 'f', 2);
}

QString SystemInfo::getDesktopEnvironment()
{
    QString xdgDesktop = qgetenv("XDG_CURRENT_DESKTOP");
    if (!xdgDesktop.isEmpty())
    {
        return xdgDesktop;
    }

    QString desktopSession = qgetenv("DESKTOP_SESSION");
    if (!desktopSession.isEmpty())
    {
        return desktopSession;
    }

    return "Unknown";
}

QPixmap SystemInfo::getDesktopIcon()
{
    QString iconPath = "/usr/share/icons/hicolor/64x64/apps/apport.png";
    if (QFile::exists(iconPath))
    {
        return QPixmap(iconPath);
    }

    // if (QFile::exists(":/icons/desktop.png"))
    // {
    //     return QPixmap(":/icons/desktop.png");
    // }

    // return QPixmap::fromImage(QImage(48, 48, QImage::Format_RGB32).fill(Qt::gray));
}
