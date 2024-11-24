#ifndef CPUUSAGEGRAPH_H
#define CPUUSAGEGRAPH_H

#include <QWidget>
#include "qcustomplot.h"
#include <vector>

class CPUUsageGraph : public QWidget {
    Q_OBJECT

public:
    explicit CPUUsageGraph(QWidget *parent = nullptr);

private slots:
    void updateGraph();

private:
    QCustomPlot *customPlot;
    int elapsedTime;
    int numCores;              


    double getCPUUsage(int coreIndex);
    std::vector<double> getAllCPUUsages();
    int getNumCores();
    QColor generateColor(int index, int total);

    void initializeGraphs();
};

#endif // CPUUSAGEGRAPH_H
