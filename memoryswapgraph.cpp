#include "memoryswapgraph.h"
#include <QVBoxLayout>
#include <QTimer>
#include <fstream>
#include <string>
#include <sstream>
#include <QDebug>

MemorySwapGraph::MemorySwapGraph(QWidget *parent)
    : QWidget(parent), updateTimer(new QTimer(this)) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    customPlot = new QCustomPlot(this);
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::blue));
    customPlot->addGraph();
    customPlot->graph(1)->setPen(QPen(Qt::red));
    customPlot->xAxis->setLabel("Time (s)");
    customPlot->yAxis->setLabel("Usage (%)");
    customPlot->xAxis->setRange(0, 100);
    customPlot->yAxis->setRange(0, 100);

    layout->addWidget(customPlot);

    memoryInfoLabel = new QLabel("Memory: Initializing...", this);
    swapInfoLabel = new QLabel("Swap: Initializing...", this);
    layout->addWidget(memoryInfoLabel);
    layout->addWidget(swapInfoLabel);

    setLayout(layout);

    connect(updateTimer, &QTimer::timeout, this, &MemorySwapGraph::updateGraph);
    updateTimer->start(1000);
}

void MemorySwapGraph::updateGraph() {
    double memoryUsage = 0.0, swapUsage = 0.0;

    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        long totalMemory = 0, freeMemory = 0, availableMemory = 0, totalSwap = 0, freeSwap = 0;

        while (std::getline(meminfo, line)) {
            std::istringstream iss(line);
            std::string key;
            long value;
            std::string unit;
            iss >> key >> value >> unit;

            if (key == "MemTotal:") totalMemory = value;
            if (key == "MemAvailable:") availableMemory = value;
            if (key == "SwapTotal:") totalSwap = value;
            if (key == "SwapFree:") freeSwap = value;
        }
        meminfo.close();

        if (totalMemory > 0)
            memoryUsage = 100.0 * (1.0 - (double)availableMemory / totalMemory);

        if (totalSwap > 0)
            swapUsage = 100.0 * (1.0 - (double)freeSwap / totalSwap);
    }

    static int elapsedTime = 0;
    customPlot->graph(0)->addData(elapsedTime, memoryUsage);
    customPlot->graph(1)->addData(elapsedTime, swapUsage);
    customPlot->xAxis->setRange(elapsedTime - 100, elapsedTime);
    customPlot->replot();

    elapsedTime++;

    updateMemorySwapInfo();
}

void MemorySwapGraph::updateMemorySwapInfo() {
    std::ifstream meminfo("/proc/meminfo");
    QString memoryText = "<b>Memory Usage:</b><br>";
    QString swapText = "<b>Swap Usage:</b><br>";

    if (meminfo.is_open()) {
        std::string line;
        long totalMemory = 0, freeMemory = 0, availableMemory = 0, totalSwap = 0, freeSwap = 0;

        while (std::getline(meminfo, line)) {
            std::istringstream iss(line);
            std::string key;
            long value;
            std::string unit;
            iss >> key >> value >> unit;

            if (key == "MemTotal:") totalMemory = value;
            if (key == "MemAvailable:") availableMemory = value;
            if (key == "SwapTotal:") totalSwap = value;
            if (key == "SwapFree:") freeSwap = value;
        }
        meminfo.close();

        memoryText += QString("Total: %1 KB<br>Available: %2 KB<br>")
                          .arg(totalMemory)
                          .arg(availableMemory);

        swapText += QString("Total: %1 KB<br>Free: %2 KB<br>")
                        .arg(totalSwap)
                        .arg(freeSwap);
    }

    memoryInfoLabel->setText(memoryText);
    swapInfoLabel->setText(swapText);
}
