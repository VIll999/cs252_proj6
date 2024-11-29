#include <QTextEdit>

#include "resourcetab.h"
#include "systeminfo.h"

ResourceTab::ResourceTab(QWidget *parent) : QWidget(parent) {
    tabWidget = new QTabWidget(this);

    // CPU tab
    CPUUsageGraph *cpuTab = new CPUUsageGraph(this);
    tabWidget->addTab(cpuTab, "CPU Usage");

    // Memory and Swap tab
    MemorySwapGraph *memoryTab = new MemorySwapGraph(this);
    tabWidget->addTab(memoryTab, "Memory & Swap");

    // Network tab
    NetworkUsageGraph *networkTab = new NetworkUsageGraph(this);
    tabWidget->addTab(networkTab, "Network Usage");

    // System Info tab
    QWidget *systemInfoTab = new QWidget(this);
    QVBoxLayout *systemInfoLayout = new QVBoxLayout(systemInfoTab);

    // Display system information
    QTextEdit *systemInfoDisplay = new QTextEdit(systemInfoTab);
    systemInfoDisplay->setReadOnly(true);
    systemInfoDisplay->setText(
        QString(
            "System Information:\n"
            "%1\n"
            "%2\n"
            "%3\n"
            "%4\n"
            "%5\n"
        ).arg(
            SystemInfo::getOSReleaseVersion(),
            SystemInfo::getKernelVersion(),
            SystemInfo::getProcessorVersion(),
            SystemInfo::getMemoryInfo(),
            SystemInfo::getDiskStorageInfo()
        )
    );

    systemInfoLayout->addWidget(systemInfoDisplay);
    systemInfoTab->setLayout(systemInfoLayout);
    tabWidget->addTab(systemInfoTab, "System Info");

    // Set layout for the ResourceTab
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabWidget);
    setLayout(layout);
}
