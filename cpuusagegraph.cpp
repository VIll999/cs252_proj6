#include "cpuusagegraph.h"
#include <QVBoxLayout>
#include <QTimer>
#include <fstream>
#include <string>

CPUUsageGraph::CPUUsageGraph(QWidget *parent) : QWidget(parent), elapsedTime(0) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    customPlot = new QCustomPlot(this);

    layout->addWidget(customPlot);
    setLayout(layout);

    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::blue));
    customPlot->xAxis->setLabel("Time");
    customPlot->yAxis->setLabel("CPU Usage (%)");
    customPlot->xAxis->setRange(0, 100);
    customPlot->yAxis->setRange(0, 100);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CPUUsageGraph::updateGraph);
    timer->start(1000); // Update every second
}

void CPUUsageGraph::updateGraph() {
    double cpuUsage = getCPUUsage();
    elapsedTime++;

    customPlot->graph(0)->addData(elapsedTime, cpuUsage);

    customPlot->xAxis->setRange(elapsedTime - 100, elapsedTime);
    customPlot->replot();
}

double CPUUsageGraph::getCPUUsage() {
    static long long lastTotal = 0, lastIdle = 0;

    std::ifstream file("/proc/stat");
    std::string line;
    getline(file, line);
    file.close();

    long long user, nice, system, idle, iowait, irq, softirq, steal;
    sscanf(line.c_str(), "cpu  %lld %lld %lld %lld %lld %lld %lld %lld", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

    long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    long long totalDiff = total - lastTotal;
    long long idleDiff = idle - lastIdle;

    lastTotal = total;
    lastIdle = idle;

    return 100.0 * (1.0 - (double)idleDiff / totalDiff);
}
