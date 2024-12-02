#include "memoryswapgraph.h"
#include <QVBoxLayout>
#include <QTimer>
#include <fstream>
#include <string>
#include <sstream>
#include <QtCharts/QPieSlice>
#include <QtCharts/QChartView>

MemorySwapGraph::MemorySwapGraph(QWidget *parent)
    : QWidget(parent), updateTimer(new QTimer(this))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    customPlot = new QCustomPlot(this);
    customPlot->addGraph();
    QPen memoryPen(Qt::blue);
    memoryPen.setWidth(3);
    customPlot->graph(0)->setPen(memoryPen);
    customPlot->addGraph();
    QPen swapPen(Qt::red);
    swapPen.setWidth(4);
    customPlot->graph(1)->setPen(swapPen);
    customPlot->xAxis->setLabel("Time (s)");
    customPlot->yAxis->setLabel("Usage (%)");
    customPlot->xAxis->setRange(0, 100);
    customPlot->yAxis->setRange(0, 100);
    customPlot->setMinimumHeight(200);
    mainLayout->addWidget(customPlot);

    QHBoxLayout *pieChartsLayout = new QHBoxLayout();

    memorySeries = new QtCharts::QPieSeries(this);
    memorySeries->append("Used", 1);
    memorySeries->append("Free", 99);
    memoryChart = new QtCharts::QChart();
    memoryChart->addSeries(memorySeries);
    memoryChart->setTitle("Memory Usage");
    memoryChartView = new QtCharts::QChartView(memoryChart);
    memoryChartView->setRenderHint(QPainter::Antialiasing);
    memoryChartView->setMinimumWidth(200);
    pieChartsLayout->addWidget(memoryChartView);

    swapSeries = new QtCharts::QPieSeries(this);
    swapSeries->append("Used", 1);
    swapSeries->append("Free", 99);
    swapChart = new QtCharts::QChart();
    swapChart->addSeries(swapSeries);
    swapChart->setTitle("Swap Usage");
    swapChartView = new QtCharts::QChartView(swapChart);
    swapChartView->setRenderHint(QPainter::Antialiasing);
    swapChartView->setMinimumWidth(200);
    pieChartsLayout->addWidget(swapChartView);

    mainLayout->addLayout(pieChartsLayout);

    setLayout(mainLayout);

    connect(updateTimer, &QTimer::timeout, this, &MemorySwapGraph::updateGraph);
    updateTimer->start(1000);
}

void MemorySwapGraph::updateGraph()
{
    double memoryUsage = 0.0, swapUsage = 0.0;
    long totalMemory = 0, availableMemory = 0, totalSwap = 0, freeSwap = 0;

    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open())
    {
        std::string line;
        while (std::getline(meminfo, line))
        {
            std::istringstream iss(line);
            std::string key;
            long value;
            std::string unit;
            iss >> key >> value >> unit;

            if (key == "MemTotal:")
                totalMemory = value;
            if (key == "MemAvailable:")
                availableMemory = value;
            if (key == "SwapTotal:")
                totalSwap = value;
            if (key == "SwapFree:")
                freeSwap = value;
        }
        meminfo.close();

        if (totalMemory > 0)
            memoryUsage = 100.0 * (1.0 - (double)availableMemory / totalMemory);

        if (totalSwap > 0)
        {
            swapUsage = 100.0 * (1.0 - (double)freeSwap / totalSwap);
        }
        else
        {
            qDebug() << "Warning: SwapTotal is zero or missing, swap usage cannot be calculated.";
            swapUsage = 0.0;
        }
    }
    else
    {
        qDebug() << "Error: Unable to open /proc/meminfo.";
    }

    static int elapsedTime = 0;

    customPlot->graph(0)->addData(elapsedTime, memoryUsage);
    customPlot->graph(1)->addData(elapsedTime, swapUsage);

    double maxSwapValue = std::max(10.0, swapUsage);
    customPlot->yAxis->setRange(0, std::max(100.0, maxSwapValue));
    customPlot->xAxis->setRange(elapsedTime - 100, elapsedTime);
    customPlot->replot();

    elapsedTime++;

    updatePieChartTitlesAndLabels(totalMemory, availableMemory, totalSwap, freeSwap);
}

void MemorySwapGraph::updatePieChartTitlesAndLabels(long totalMemory, long availableMemory, long totalSwap, long freeSwap)
{

    memoryChart->setTitle(QString("Total Memory: %1 KB").arg(totalMemory));
    memorySeries->clear();
    long usedMemory = totalMemory - availableMemory;
    QtCharts::QPieSlice *usedMemorySlice = memorySeries->append(QString("Used: %1 KB").arg(usedMemory), 100.0 * usedMemory / totalMemory);
    QtCharts::QPieSlice *freeMemorySlice = memorySeries->append(QString("Free: %1 KB").arg(availableMemory), 100.0 * availableMemory / totalMemory);

    usedMemorySlice->setBrush(Qt::blue);
    freeMemorySlice->setBrush(Qt::gray);

    swapChart->setTitle(QString("Total Swap: %1 KB").arg(totalSwap));
    swapSeries->clear();
    long usedSwap = totalSwap - freeSwap;

    if (totalSwap > 0)
    {
        QtCharts::QPieSlice *usedSwapSlice = swapSeries->append(QString("Used: %1 KB").arg(usedSwap), 100.0 * usedSwap / totalSwap);
        QtCharts::QPieSlice *freeSwapSlice = swapSeries->append(QString("Free: %1 KB").arg(freeSwap), 100.0 * freeSwap / totalSwap);

        usedSwapSlice->setBrush(Qt::red);
        freeSwapSlice->setBrush(Qt::gray);
    }
    else
    {

        swapSeries->append("No Swap Available", 100.0)->setBrush(Qt::gray);
    }

    memoryChart->update();
    swapChart->update();
}

void MemorySwapGraph::updateMemorySwapInfo()
{
    std::ifstream meminfo("/proc/meminfo");

    if (meminfo.is_open())
    {
        std::string line;
        long totalMemory = 0, availableMemory = 0, totalSwap = 0, freeSwap = 0;

        while (std::getline(meminfo, line))
        {
            std::istringstream iss(line);
            std::string key;
            long value;
            std::string unit;
            iss >> key >> value >> unit;

            if (key == "MemTotal:")
                totalMemory = value;
            if (key == "MemAvailable:")
                availableMemory = value;
            if (key == "SwapTotal:")
                totalSwap = value;
            if (key == "SwapFree:")
                freeSwap = value;
        }
        meminfo.close();

        memoryChart->setTitle(QString("Total Memory: %1 KB").arg(totalMemory));
        swapChart->setTitle(QString("Total Swap: %1 KB").arg(totalSwap));

        memorySeries->clear();
        memorySeries->append(QString("Used: %1 KB").arg(totalMemory - availableMemory),
                             100.0 * (1.0 - (double)availableMemory / totalMemory));
        memorySeries->append(QString("Free: %1 KB").arg(availableMemory),
                             100.0 * (double)availableMemory / totalMemory);

        swapSeries->clear();
        if (totalSwap > 0)
        {
            swapSeries->append(QString("Used: %1 KB").arg(totalSwap - freeSwap),
                               100.0 * (1.0 - (double)freeSwap / totalSwap));
            swapSeries->append(QString("Free: %1 KB").arg(freeSwap),
                               100.0 * (double)freeSwap / totalSwap);
        }
        else
        {

            swapSeries->append("No Swap Available", 100.0);
        }

        memoryChart->update();
        swapChart->update();
    }
}

void MemorySwapGraph::updatePieCharts(double memoryUsage, double swapUsage)
{

    memorySeries->clear();
    QtCharts::QPieSlice *usedMemorySlice = memorySeries->append("Used", memoryUsage);
    QtCharts::QPieSlice *freeMemorySlice = memorySeries->append("Free", 100 - memoryUsage);

    usedMemorySlice->setBrush(Qt::blue);
    freeMemorySlice->setBrush(Qt::gray);

    memoryChart->update();

    swapSeries->clear();
    QtCharts::QPieSlice *usedSwapSlice = swapSeries->append("Used", swapUsage);
    QtCharts::QPieSlice *freeSwapSlice = swapSeries->append("Free", 100 - swapUsage);

    usedSwapSlice->setBrush(Qt::red);
    freeSwapSlice->setBrush(Qt::gray);

    swapChart->update();
}
