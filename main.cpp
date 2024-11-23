#include <QApplication>
#include "cpuusagegraph.h" // Includes the CPUUsageGraph class
#include <QVBoxLayout>
#include <fstream>
#include <string>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    CPUUsageGraph window;
    window.setWindowTitle("CPU Usage Graph");
    window.resize(800, 400);
    window.show();

    return app.exec();
}
