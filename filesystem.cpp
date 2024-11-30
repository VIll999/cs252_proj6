#include "filesystem.h"
#include <QDebug>
#include <QPushButton>

FilesystemTab::FilesystemTab(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Create a table to display filesystem information
    table = new QTableWidget(this);
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({"Device", "Directory", "Type", "Total", "Free", "Available", "Used"});
    table->horizontalHeader()->setStretchLastSection(true); // Stretch the last column
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);                // Hide row headers
    table->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make table read-only
    table->setSelectionMode(QAbstractItemView::NoSelection);   // Disable selection
    table->setShowGrid(true);

    mainLayout->addWidget(table);

    // Add "Refresh" button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *refreshButton = new QPushButton("Refresh", this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(refreshButton);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    connect(refreshButton, &QPushButton::clicked, this, &FilesystemTab::populateTable);

    // Populate the table with filesystem information
    populateTable();

    // timer = new QTimer(this);
    // connect(timer, &QTimer::timeout, this, &FilesystemTab::populateTable);
    // timer->start(10000); // Update every 10 seconds
}

void FilesystemTab::populateTable()
{
    table->setRowCount(0); // Clear the table

    FILE *mounts = setmntent("/etc/mtab", "r");
    if (!mounts)
    {
        perror("setmntent");
        return;
    }

    struct mntent *ent;
    int row = 0;

    while ((ent = getmntent(mounts)) != nullptr)
    {
        struct statvfs vfs;
        if (statvfs(ent->mnt_dir, &vfs) != 0)
        {
            continue; // Skip if statvfs fails
        }

        // Filesystem details
        QString device = QString(ent->mnt_fsname);
        QString directory = QString(ent->mnt_dir);
        QString type = QString(ent->mnt_type);
        double total = vfs.f_blocks * vfs.f_frsize / (1024.0 * 1024.0 * 1024.0);       // Total size in GiB
        double free = vfs.f_bfree * vfs.f_frsize / (1024.0 * 1024.0 * 1024.0);         // Free size in GiB
        double available = vfs.f_bavail * vfs.f_frsize / (1024.0 * 1024.0 * 1024.0);   // Available size in GiB
        double used = total - free;                                                    // Used size in GiB
        int usedPercentage = (total > 0) ? static_cast<int>((used / total) * 100) : 0; // Used percentage

        // Add a new row
        table->insertRow(row);

        // Add icon + device name in the first column
        QTableWidgetItem *deviceItem = new QTableWidgetItem(QIcon::fromTheme("drive-harddisk"), device);
        table->setItem(row, 0, deviceItem);

        table->setItem(row, 1, new QTableWidgetItem(directory));
        table->setItem(row, 2, new QTableWidgetItem(type));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(total, 'f', 2) + " GiB"));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(free, 'f', 2) + " GiB"));
        table->setItem(row, 5, new QTableWidgetItem(QString::number(available, 'f', 2) + " GiB")); // Available

        // Add progress bar in the "Used" column
        QProgressBar *progressBar = new QProgressBar();
        progressBar->setValue(usedPercentage);
        progressBar->setTextVisible(true); // Show percentage text
        progressBar->setFormat(QString::number(used, 'f', 2) + " GiB");
        table->setCellWidget(row, 6, progressBar);

        row++;
    }

    endmntent(mounts); // Close the mount tabl

    qDebug() << "Filesystems table updated";
}
