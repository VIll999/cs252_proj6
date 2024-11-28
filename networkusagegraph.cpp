#include "networkusagegraph.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <fstream>
#include <sstream>
#include <string>
#include <QDebug>
#include <QLabel>
#include <algorithm>

NetworkUsageGraph::NetworkUsageGraph(QWidget *parent)
    : QWidget(parent), updateTimer(new QTimer(this)), highestDownloadRate(1000.0)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    customPlot = new QCustomPlot(this);
    customPlot->addGraph();
    QPen downloadPen(Qt::green);
    downloadPen.setWidth(3);
    customPlot->graph(0)->setPen(downloadPen);

    customPlot->addGraph();
    QPen uploadPen(Qt::blue);
    uploadPen.setWidth(3);
    customPlot->graph(1)->setPen(uploadPen);

    customPlot->xAxis->setLabel("Time (s)");
    customPlot->yAxis->setLabel("Data Rate (KB/s)");
    customPlot->xAxis->setRange(0, 100);
    customPlot->yAxis->setRange(0, 1000);
    customPlot->setMinimumHeight(200);
    mainLayout->addWidget(customPlot);

    QVBoxLayout *infoLayout = new QVBoxLayout();

    QLabel *downloadColorKey = new QLabel(this);
    downloadColorKey->setFixedSize(10, 10);
    downloadColorKey->setStyleSheet("background-color: green;");
    QLabel *downArrow = new QLabel("↓", this);
    downArrow->setStyleSheet("color: green; font-size: 22px; font-weight: bold;");
    QLabel *downloadInfo = new QLabel("Download: 0 KB/s", this);
    downloadInfo->setStyleSheet("font-size: 16px;");

    QHBoxLayout *downloadLayout = new QHBoxLayout();
    downloadLayout->addWidget(downloadColorKey);
    downloadLayout->addWidget(downArrow);
    downloadLayout->addWidget(downloadInfo);
    downloadLayout->setSpacing(1);

    QLabel *uploadColorKey = new QLabel(this);
    uploadColorKey->setFixedSize(10, 10);
    uploadColorKey->setStyleSheet("background-color: blue;");
    QLabel *upArrow = new QLabel("↑", this);
    upArrow->setStyleSheet("color: blue; font-size: 22px; font-weight: bold;");
    QLabel *uploadInfo = new QLabel("Upload: 0 KB/s", this);
    uploadInfo->setStyleSheet("font-size: 16px;");

    QHBoxLayout *uploadLayout = new QHBoxLayout();
    uploadLayout->addWidget(uploadColorKey);
    uploadLayout->addWidget(upArrow);
    uploadLayout->addWidget(uploadInfo);
    uploadLayout->setSpacing(1);

    infoLayout->addLayout(downloadLayout);
    infoLayout->addSpacing(10);
    infoLayout->addLayout(uploadLayout);

    mainLayout->addLayout(infoLayout);
    setLayout(mainLayout);

    connect(updateTimer, &QTimer::timeout, this, [=]()
            {
        this->updateGraph();


        downloadInfo->setText(QString("Download: %1 KB/s").arg(currentDownloadRate, 0, 'f', 2));
        uploadInfo->setText(QString("Upload: %1 KB/s").arg(currentUploadRate, 0, 'f', 2)); });
    updateTimer->start(1000);
}

void NetworkUsageGraph::updateGraph()
{
    static bool initialized = false;
    static long long lastReceived = 0, lastTransmitted = 0;
    long long received = 0, transmitted = 0;

    std::ifstream netinfo("/proc/net/dev");
    if (netinfo.is_open())
    {
        std::string line;

        std::getline(netinfo, line);
        std::getline(netinfo, line);

        while (std::getline(netinfo, line))
        {
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

    if (!initialized)
    {
        lastReceived = received;
        lastTransmitted = transmitted;
        initialized = true;
        return;
    }

    long long deltaReceived = received - lastReceived;
    long long deltaTransmitted = transmitted - lastTransmitted;
    lastReceived = received;
    lastTransmitted = transmitted;

    currentDownloadRate = deltaReceived / 1024.0;
    currentUploadRate = deltaTransmitted / 1024.0;

    highestDownloadRate = std::max(highestDownloadRate, currentDownloadRate);

    static int elapsedTime = 0;
    customPlot->graph(0)->addData(elapsedTime, currentDownloadRate);
    customPlot->graph(1)->addData(elapsedTime, currentUploadRate);

    customPlot->yAxis->setRange(0, std::max(1000.0, highestDownloadRate * 1.2));
    customPlot->xAxis->setRange(elapsedTime - 100, elapsedTime);
    customPlot->replot();

    elapsedTime++;
}
