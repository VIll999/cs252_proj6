#include "cpuusagegraph.h"
#include <QTimer>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <QDebug>
#include <QMouseEvent>

CPUUsageGraph::CPUUsageGraph(QWidget *parent)
    : QWidget(parent), elapsedTime(0), updateTimer(new QTimer(this))
{
    numCores = getNumCores();

    mainLayout = new QVBoxLayout(this);

    customPlot = new QCustomPlot(this);
    initializeGraphs();
    customPlot->setMinimumSize(600, 300);
    customPlot->xAxis->setLabel("Time (s)");
    customPlot->yAxis->setLabel("CPU Usage (%)");
    customPlot->xAxis->setRange(0, 100);
    customPlot->yAxis->setRange(0, 100);
    mainLayout->addWidget(customPlot);

    cpuInfoDisplay = new QTextEdit(this);
    cpuInfoDisplay->setReadOnly(true);
    cpuInfoDisplay->setMinimumHeight(150);
    mainLayout->addWidget(cpuInfoDisplay);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    QLabel *inputLabel = new QLabel("Select Core (Index):", this);
    coreInput = new QLineEdit(this);
    QPushButton *selectButton = new QPushButton("Select", this);
    inputLayout->addWidget(inputLabel);
    inputLayout->addWidget(coreInput);
    inputLayout->addWidget(selectButton);
    mainLayout->addLayout(inputLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    startButton = new QPushButton("Start", this);
    stopButton = new QPushButton("Stop", this);
    resetButton = new QPushButton("Reset", this);
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(resetButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    connect(startButton, &QPushButton::clicked, this, &CPUUsageGraph::startUpdates);
    connect(stopButton, &QPushButton::clicked, this, &CPUUsageGraph::stopUpdates);
    connect(resetButton, &QPushButton::clicked, this, &CPUUsageGraph::resetGraph);
    connect(selectButton, &QPushButton::clicked, this, &CPUUsageGraph::selectCore);

    connect(updateTimer, &QTimer::timeout, this, &CPUUsageGraph::updateGraph);
    updateTimer->start(1000);
}

void CPUUsageGraph::initializeGraphs()
{
    for (int i = 0; i < numCores; ++i)
    {
        customPlot->addGraph();
        customPlot->graph(i)->setPen(QPen(generateColor(i, numCores)));
    }
}

void CPUUsageGraph::updateGraph()
{
    std::vector<double> cpuUsages = getAllCPUUsages();
    elapsedTime++;

    for (size_t i = 0; i < cpuUsages.size(); ++i)
    {
        customPlot->graph(i)->addData(elapsedTime, cpuUsages[i]);
    }

    customPlot->xAxis->setRange(elapsedTime - 100, elapsedTime);
    customPlot->replot();

    updateCPUInfo(cpuUsages);
}

void CPUUsageGraph::updateCPUInfo(const std::vector<double> &cpuUsages)
{

    int scrollPosition = cpuInfoDisplay->verticalScrollBar()->value();

    QString info = "<b>CPU Usage:</b><br>";
    for (size_t i = 0; i < cpuUsages.size(); ++i)
    {
        info += QString("<span style='color:%1;'>Core %2: %3%</span><br>")
                    .arg(generateColor(i, numCores).name())
                    .arg(i)
                    .arg(cpuUsages[i], 0, 'f', 1);
    }

    cpuInfoDisplay->setHtml(info);

    cpuInfoDisplay->verticalScrollBar()->setValue(scrollPosition);
}

void CPUUsageGraph::selectCore()
{
    bool ok;
    int coreIndex = coreInput->text().toInt(&ok);

    if (ok && coreIndex >= 0 && coreIndex < numCores)
    {
        selectedCore = coreIndex;
        filterGraphByCore(coreIndex);
    }
    else
    {
        coreInput->setText("Invalid");
    }
}

void CPUUsageGraph::filterGraphByCore(int coreIndex)
{
    for (int i = 0; i < customPlot->graphCount(); ++i)
    {
        customPlot->graph(i)->setVisible(i == coreIndex);
    }
    QString newTitle = QString("CPU %1 Usage (%)").arg(coreIndex);
    customPlot->yAxis->setLabel(newTitle);
    customPlot->replot();
}

void CPUUsageGraph::resetGraph()
{
    selectedCore = -1;
    for (int i = 0; i < customPlot->graphCount(); ++i)
    {
        customPlot->graph(i)->setVisible(true);
    }
    customPlot->yAxis->setLabel("CPU Usage (%)");
    customPlot->replot();
}

void CPUUsageGraph::startUpdates()
{
    if (!updateTimer->isActive())
    {
        updateTimer->start(1000);
    }
}

void CPUUsageGraph::stopUpdates()
{
    if (updateTimer->isActive())
    {
        updateTimer->stop();
    }
}

int CPUUsageGraph::getNumCores()
{
    std::ifstream file("/proc/stat");
    int coreCount = -1;
    std::string line;

    while (std::getline(file, line))
    {
        if (line.find("cpu") == 0 && line != "cpu")
        {
            ++coreCount;
        }
    }
    file.close();
    return coreCount;
}

std::vector<double> CPUUsageGraph::getAllCPUUsages()
{
    int numCores = getNumCores();
    std::vector<double> usages;

    for (int i = 0; i < numCores; ++i)
    {
        usages.push_back(getCPUUsage(i));
    }

    return usages;
}

double CPUUsageGraph::getCPUUsage(int coreIndex)
{
    static std::vector<long long> lastTotal(numCores, 0);
    static std::vector<long long> lastIdle(numCores, 0);

    std::ifstream file("/proc/stat");
    std::string line;
    for (int i = 0; i <= coreIndex; ++i)
    {
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

    return 100.0 * (1.0 - (double)idleDiff / totalDiff);
}

QColor CPUUsageGraph::generateColor(int index, int total)
{
    int hue = static_cast<int>((360.0 * index) / total) % 360;
    return QColor::fromHsv(hue, 255, 255);
}
