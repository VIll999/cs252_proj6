#include "systeminfo.h"
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QDebug>

QString SystemInfo::getOSReleaseVersion() {
    QFile file("/etc/os-release");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "OS Version: Unknown";

    QTextStream in(&file);
    QString line;
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.startsWith("PRETTY_NAME=")) {
            file.close();
            return line.mid(line.indexOf("=") + 1).remove("\"").trimmed();
        }
    }
    file.close();
    return "OS Version: Unknown";
}

QString SystemInfo::getKernelVersion() {
    QProcess process;
    process.start("uname", {"-r"});
    if (!process.waitForFinished())
        return "Kernel Version: Unknown";

    return QString(process.readAllStandardOutput()).trimmed();
}

QString SystemInfo::getProcessorVersion() {
    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "Processor: Unknown";

    QTextStream in(&file);
    QString line;
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.startsWith("model name")) {
            file.close();
            return QString("Processor: %1").arg(line.mid(line.indexOf(":") + 2).trimmed());
        }
    }
    file.close();
    return "Processor: Unknown";
}

QString SystemInfo::getMemoryInfo() {
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "Memory: Unknown";

    QTextStream in(&file);
    QString totalMemory, availableMemory;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("MemTotal:")) {
            totalMemory = line.split(":")[1].trimmed();
        } else if (line.startsWith("MemAvailable:")) {
            availableMemory = line.split(":")[1].trimmed();
        }
    }
    file.close();

    if (totalMemory.isEmpty() || availableMemory.isEmpty())
        return "Memory: Unknown";

    return QString("Total Memory: %1\nAvailable Memory: %2").arg(totalMemory, availableMemory);
}

QString SystemInfo::getDiskStorageInfo() {
    QProcess process;
    process.start("df", {"-h", "/"});
    if (!process.waitForFinished())
        return "Disk Storage: Unknown";

    QString output = QString(process.readAllStandardOutput());
    QStringList lines = output.split("\n");
    if (lines.size() > 1) {
        QString diskLine = lines[1].simplified();
        QStringList diskInfo = diskLine.split(" ");
        if (diskInfo.size() >= 5) {
            return QString("Disk Storage: %1 Total, %2 Used, %3 Free (%4)").arg(diskInfo[1], diskInfo[2], diskInfo[3], diskInfo[4]);
        }
    }
    return "Disk Storage: Unknown";
}
