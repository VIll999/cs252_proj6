QT += widgets core gui printsupport charts

SOURCES += \
    main.cpp \
    cpuusagegraph.cpp \
    qcustomplot.cpp \
    memoryswapgraph.cpp \
    networkusagegraph.cpp \
    resourcetab.cpp \
    systeminfo.cpp

HEADERS += \
    cpuusagegraph.h \
    qcustomplot.h \
    memoryswapgraph.h \
    networkusagegraph.h \
    resourcetab.h \
    systeminfo.h

INCLUDEPATH += .
CONFIG += c++17
