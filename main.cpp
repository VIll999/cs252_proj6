#include <QApplication>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include "resourcetab.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget mainWindow;
    mainWindow.setWindowTitle("Task Manager");
    mainWindow.resize(800, 600);

    QTabWidget *mainTabWidget = new QTabWidget(&mainWindow);

    // Add Resource tab
    ResourceTab *resourceTab = new ResourceTab();
    mainTabWidget->addTab(resourceTab, "Resources");

    // Future tabs for other functionalities

    QVBoxLayout *layout = new QVBoxLayout(&mainWindow);
    layout->addWidget(mainTabWidget);
    mainWindow.setLayout(layout);

    mainWindow.show();
    return app.exec();
}
