#include "cpuusagegraph.h"
#include <QVBoxLayout>
#include <QTimer>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <QDebug>

QColor CPUUsageGraph::generateColor(int index, int total) {
    int hue = static_cast<int>((360.0 * index) / total) % 360;
    return QColor::fromHsv(hue, 255, 255);
}

void CPUUsageGraph::initializeGraphs() {
    numCores = getNumCores();
    for (int i = 0; i < numCores; ++i) {
        customPlot->addGraph();
        QColor color = generateColor(i, numCores);
        if (!color.isValid()) {
            qDebug() << "Color generation failed for index:" << i;
            continue;
        }
        customPlot->graph(i)->setPen(QPen(color));
    }
}

CPUUsageGraph::CPUUsageGraph(QWidget *parent) : QWidget(parent), elapsedTime(0), numCores(0) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    customPlot = new QCustomPlot(this);

    layout->addWidget(customPlot);
    setLayout(layout);

    customPlot->xAxis->setLabel("Time");
    customPlot->yAxis->setLabel("CPU Usage (%)");
    customPlot->xAxis->setRange(0, 100);
    customPlot->yAxis->setRange(0, 100);

    initializeGraphs();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CPUUsageGraph::updateGraph);
    timer->start(1000);
}

void CPUUsageGraph::updateGraph() {
    std::vector<double> cpuUsages = getAllCPUUsages();
    elapsedTime++;

    for (size_t i = 0; i < cpuUsages.size(); ++i) {
        customPlot->graph(i)->addData(elapsedTime, cpuUsages[i]);
    }

    customPlot->xAxis->setRange(elapsedTime - 100, elapsedTime);
    customPlot->replot();
}

double CPUUsageGraph::getCPUUsage(int coreIndex) {
    static std::vector<long long> lastTotal(getNumCores(), 0);
    static std::vector<long long> lastIdle(getNumCores(), 0);

    std::ifstream file("/proc/stat");
    std::string line;
    for (int i = 0; i <= coreIndex; ++i) {
        std::getline(file, line);
    }
    file.close();

    long long user, nice, system, idle, iowait, irq, softirq, steal;
    sscanf(line.c_str(), "cpu%*d %lld %lld %lld %lld %lld %lld %lld %lld",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

    long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    long long totalDiff = total - lastTotal[coreIndex];
    long long idleDiff = idle - lastIdle[coreIndex];

    lastTotal[coreIndex] = total;
    lastIdle[coreIndex] = idle;

    return totalDiff > 0 ? 100.0 * (1.0 - (double)idleDiff / totalDiff) : 0.0;
}

std::vector<double> CPUUsageGraph::getAllCPUUsages() {
    int numCores = getNumCores();
    std::vector<double> usages;

    for (int i = 0; i < numCores; ++i) {
        usages.push_back(getCPUUsage(i));
    }

    return usages;
}

int CPUUsageGraph::getNumCores() {
    std::ifstream file("/proc/stat");
    int coreCount = -1;
    std::string line;

    while (std::getline(file, line)) {
        if (line.find("cpu") == 0 && line != "cpu") {
            ++coreCount;
        }
    }
    file.close();
    return coreCount;
}
