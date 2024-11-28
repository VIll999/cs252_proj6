#ifndef MEMORYSWAPGRAPH_H
#define MEMORYSWAPGRAPH_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include "qcustomplot.h"

class MemorySwapGraph : public QWidget
{
    Q_OBJECT

public:
    explicit MemorySwapGraph(QWidget *parent = nullptr);

private slots:
    void updateGraph();

private:
    QCustomPlot *customPlot;
    QtCharts::QChart *memoryChart;
    QtCharts::QPieSeries *memorySeries;
    QtCharts::QChartView *memoryChartView;

    QtCharts::QChart *swapChart;
    QtCharts::QPieSeries *swapSeries;
    QtCharts::QChartView *swapChartView;

    QTimer *updateTimer;

    void updateMemorySwapInfo();
    void updatePieCharts(double memoryUsage, double swapUsage);
    void updatePieChartTitlesAndLabels(long totalMemory, long availableMemory, long totalSwap, long freeSwap);
};

#endif // MEMORYSWAPGRAPH_H
