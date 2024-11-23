#ifndef CPUUSAGEGRAPH_H
#define CPUUSAGEGRAPH_H

#include <QWidget>
#include "qcustomplot.h"

class CPUUsageGraph : public QWidget {
    Q_OBJECT

public:
    explicit CPUUsageGraph(QWidget *parent = nullptr);

private slots:
    void updateGraph();

private:
    QCustomPlot *customPlot;
    int elapsedTime;

    double getCPUUsage();
};

#endif // CPUUSAGEGRAPH_H
