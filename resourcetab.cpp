#include <QTextEdit>

#include "resourcetab.h"
#include "systeminfo.h"

ResourceTab::ResourceTab(QWidget *parent) : QWidget(parent)
{
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

    // Set layout for the ResourceTab
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabWidget);
    setLayout(layout);
}
