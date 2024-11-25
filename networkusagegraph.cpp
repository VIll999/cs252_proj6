#include "networkusagegraph.h"
#include <QVBoxLayout>
#include <QTimer>
#include <fstream>
#include <sstream>
#include <string>
#include <QDebug>

NetworkUsageGraph::NetworkUsageGraph(QWidget *parent)
    : QWidget(parent), updateTimer(new QTimer(this)) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    customPlot = new QCustomPlot(this);
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::green));
    customPlot->addGraph();
    customPlot->graph(1)->setPen(QPen(Qt::blue));
    customPlot->xAxis->setLabel("Time (s)");
    customPlot->yAxis->setLabel("Data Rate (KB/s)");
    customPlot->xAxis->setRange(0, 100);
    customPlot->yAxis->setRange(0, 1000);

    layout->addWidget(customPlot);

    networkInfoLabel = new QLabel("Network: Initializing...", this);
    layout->addWidget(networkInfoLabel);

    setLayout(layout);

    connect(updateTimer, &QTimer::timeout, this, &NetworkUsageGraph::updateGraph);
    updateTimer->start(1000);
}

void NetworkUsageGraph::updateGraph() {
    static long long lastReceived = 0, lastTransmitted = 0;
    long long received = 0, transmitted = 0;

    std::ifstream netinfo("/proc/net/dev");
    if (netinfo.is_open()) {
        std::string line;

        std::getline(netinfo, line);
        std::getline(netinfo, line);

        while (std::getline(netinfo, line)) {
            std::istringstream iss(line);
            std::string interfaceName;
            long long recv, transmit;
            iss >> interfaceName >> recv;
            iss.ignore(std::numeric_limits<std::streamsize>::max(), ':');
            iss >> transmit;

            received += recv;
            transmitted += transmit;
        }
        netinfo.close();
    }

    long long deltaReceived = received - lastReceived;
    long long deltaTransmitted = transmitted - lastTransmitted;
    lastReceived = received;
    lastTransmitted = transmitted;

    double downloadRate = deltaReceived / 1024.0;
    double uploadRate = deltaTransmitted / 1024.0;

    static int elapsedTime = 0;
    customPlot->graph(0)->addData(elapsedTime, downloadRate);
    customPlot->graph(1)->addData(elapsedTime, uploadRate);
    customPlot->xAxis->setRange(elapsedTime - 100, elapsedTime);
    customPlot->replot();

    elapsedTime++;

    QString info = QString("<b>Network Usage:</b><br>Download: %1 KB/s<br>Upload: %2 KB/s")
                       .arg(downloadRate, 0, 'f', 2)
                       .arg(uploadRate, 0, 'f', 2);
    networkInfoLabel->setText(info);
}
