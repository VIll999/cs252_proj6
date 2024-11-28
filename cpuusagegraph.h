#ifndef CPUUSAGEGRAPH_H
#define CPUUSAGEGRAPH_H

#include <QWidget>
#include "qcustomplot.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QTextEdit>

class CPUUsageGraph : public QWidget
{
    Q_OBJECT

public:
    explicit CPUUsageGraph(QWidget *parent = nullptr);

private slots:
    void updateGraph();
    void startUpdates();
    void stopUpdates();
    void resetGraph();
    void selectCore();

private:
    QCustomPlot *customPlot;
    QTextEdit *cpuInfoDisplay;
    QLineEdit *coreInput;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *resetButton;
    QVBoxLayout *mainLayout;

    QTimer *updateTimer;
    int elapsedTime;
    int numCores;
    int selectedCore;

    double getCPUUsage(int coreIndex);
    std::vector<double> getAllCPUUsages();
    int getNumCores();
    QColor generateColor(int index, int total);

    void initializeGraphs();
    void updateCPUInfo(const std::vector<double> &cpuUsages);
    void filterGraphByCore(int coreIndex);
};

#endif // CPUUSAGEGRAPH_H
