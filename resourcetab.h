#ifndef RESOURCETAB_H
#define RESOURCETAB_H

#include <QWidget>
#include <QTabWidget>

#include "cpuusagegraph.h"
#include "memoryswapgraph.h"
#include "networkusagegraph.h"
#include "systeminfo.h"

class ResourceTab : public QWidget {
    Q_OBJECT

public:
    explicit ResourceTab(QWidget *parent = nullptr);

private:
    QTabWidget *tabWidget;
};

#endif // RESOURCETAB_H
