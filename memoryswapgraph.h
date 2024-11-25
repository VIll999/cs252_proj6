#ifndef MEMORYSWAPGRAPH_H
#define MEMORYSWAPGRAPH_H

#include <QWidget>
#include "qcustomplot.h"

class MemorySwapGraph : public QWidget {
    Q_OBJECT

public:
    explicit MemorySwapGraph(QWidget *parent = nullptr);

private slots:
    void updateGraph();

private:
    QCustomPlot *customPlot;
    QLabel *memoryInfoLabel;
    QLabel *swapInfoLabel;
    QTimer *updateTimer;

    void updateMemorySwapInfo();
};

#endif // MEMORYSWAPGRAPH_H
