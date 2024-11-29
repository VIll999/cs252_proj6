#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QString>

class SystemInfo {
public:
    static QString getOSReleaseVersion();
    static QString getKernelVersion();
    static QString getProcessorVersion();
    static QString getMemoryInfo();
    static QString getDiskStorageInfo();
};

#endif // SYSTEMINFO_H
