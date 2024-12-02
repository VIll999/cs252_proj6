#include <QApplication>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include "resourcetab.h"
#include "systeminfo.h"
#include "filesystem.h"
#include "processmonitor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget mainWindow;
    mainWindow.setWindowTitle("Task Manager");
    mainWindow.resize(800, 600);

    QTabWidget *mainTabWidget = new QTabWidget(&mainWindow);

    SystemInfo *systemInfoTab = new SystemInfo();
    mainTabWidget->addTab(systemInfoTab, "System Info");

    ProcessMonitor *processTab = new ProcessMonitor();
    mainTabWidget->addTab(processTab, "Processes");

    ResourceTab *resourceTab = new ResourceTab();
    mainTabWidget->addTab(resourceTab, "Resources");

    FilesystemTab *filesystemTab = new FilesystemTab();
    mainTabWidget->addTab(filesystemTab, "Filesystems");

    QVBoxLayout *layout = new QVBoxLayout(&mainWindow);
    layout->addWidget(mainTabWidget);
    mainWindow.setLayout(layout);

    mainWindow.show();
    return app.exec();
}
