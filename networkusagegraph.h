#ifndef NETWORKUSAGEGRAPH_H
#define NETWORKUSAGEGRAPH_H

#include <QWidget>
#include "qcustomplot.h"

class NetworkUsageGraph : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkUsageGraph(QWidget *parent = nullptr);

private slots:
    void updateGraph();

private:
    QCustomPlot *customPlot;
    QLabel *networkInfoLabel;
    QTimer *updateTimer;

    void updateNetworkInfo();
    double currentDownloadRate = 0.0;
    double currentUploadRate = 0.0;
    double highestDownloadRate = 0.0;
};

#endif // NETWORKUSAGEGRAPH_H
