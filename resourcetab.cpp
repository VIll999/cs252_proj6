#include <QTextEdit>

#include "resourcetab.h"
#include "systeminfo.h"

ResourceTab::ResourceTab(QWidget *parent) : QWidget(parent)
{
    tabWidget = new QTabWidget(this);

    // System tab
    QWidget *systemInfoTab = new SystemInfo(this);
    tabWidget->addTab(systemInfoTab, "System");

    // CPU tab
    CPUUsageGraph *cpuTab = new CPUUsageGraph(this);
    tabWidget->addTab(cpuTab, "CPU Usage");

    // Memory and Swap tab
    MemorySwapGraph *memoryTab = new MemorySwapGraph(this);
    tabWidget->addTab(memoryTab, "Memory & Swap");

    // Network tab
    NetworkUsageGraph *networkTab = new NetworkUsageGraph(this);
    tabWidget->addTab(networkTab, "Network Usage");

    // Filesystem tab
    FilesystemTab *filesystemTab = new FilesystemTab(this);
    tabWidget->addTab(filesystemTab, "Filesystems");

    // Set layout for the ResourceTab
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabWidget);
    setLayout(layout);
}
