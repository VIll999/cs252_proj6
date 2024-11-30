#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>
#include <QProgressBar>
#include <QTableWidgetItem>
#include <QIcon>
#include <QTimer>
#include <sys/statvfs.h>
#include <mntent.h>

class FilesystemTab : public QWidget
{
    Q_OBJECT

public:
    FilesystemTab(QWidget *parent = 0);
    ~FilesystemTab()
    {
        delete table;
        // delete timer;
    }

private:
    void populateTable();

    QTableWidget *table;
    // QTimer *timer;
};

#endif // FILESYSTEM_H
