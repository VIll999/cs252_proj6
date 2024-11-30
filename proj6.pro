QT += widgets core gui printsupport charts

SOURCES += \
    main.cpp \
    cpuusagegraph.cpp \
    qcustomplot.cpp \
    memoryswapgraph.cpp \
    networkusagegraph.cpp \
    resourcetab.cpp \
    systeminfo.cpp \
    filesystem.cpp

HEADERS += \
    cpuusagegraph.h \
    qcustomplot.h \
    memoryswapgraph.h \
    networkusagegraph.h \
    resourcetab.h \
    systeminfo.h \
    filesystem.h

INCLUDEPATH += .
CONFIG += c++17