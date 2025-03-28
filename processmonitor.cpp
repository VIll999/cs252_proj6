#include "processmonitor.h"

ProcessMonitor::ProcessMonitor(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Add load averages label
    QHBoxLayout *loadLayout = new QHBoxLayout();
    loadLabel = new QLabel("Load averages for last 1, 5, 15 minutes: ...", this);
    QPushButton *refreshButton = new QPushButton("Refresh", this);
    loadLayout->addWidget(loadLabel);
    loadLayout->addWidget(refreshButton);
    loadLayout->addStretch(); // Push the button and label to the left
    mainLayout->addLayout(loadLayout);

    // Create process tree
    processTree = new QTreeWidget(this);
    processTree->setHeaderLabels({"Process Name", "Status", "%CPU", "ID", "Memory"});
    processTree->header()->setSectionResizeMode(QHeaderView::Stretch);
    processTree->setContextMenuPolicy(Qt::CustomContextMenu); // Enable custom context menu
    mainLayout->addWidget(processTree);

    // Add "End Process" button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    endProcessButton = new QPushButton("End Process", this);
    endProcessButton->setEnabled(false); // Initially disabled
    buttonLayout->addStretch();
    buttonLayout->addWidget(endProcessButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    setWindowTitle("Process Monitor");
    // resize(800, 600);

    // Connect signals
    connect(refreshButton, &QPushButton::clicked, this, &ProcessMonitor::refreshProcesses);
    connect(processTree, &QTreeWidget::customContextMenuRequested, this, &ProcessMonitor::showContextMenu);
    connect(endProcessButton, &QPushButton::clicked, this, &ProcessMonitor::endProcess);
    connect(processTree, &QTreeWidget::itemSelectionChanged, this, &ProcessMonitor::updateButtonState);

    // Refresh processes on startup
    refreshProcesses();
}

QString ProcessMonitor::getUsername(uid_t uid)
{
    QFile file("/etc/passwd");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return "Unknown";
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList fields = line.split(":");
        if (fields.size() > 2 && fields[2].toUInt() == uid)
        {
            return fields[0]; // Username
        }
    }

    return "Unknown";
}

void ProcessMonitor::refreshProcesses()
{
    processTree->clear();

    // Update load averages
    QFile loadavgFile("/proc/loadavg");
    if (loadavgFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString loadavg = loadavgFile.readLine().trimmed();
        loadLabel->setText("Load averages for last 1, 5, 15 minutes: " + loadavg.split(" ").mid(0, 3).join(", "));
    }

    // Scan /proc for processes
    QDir procDir("/proc");
    QStringList entries = procDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QMap<int, QTreeWidgetItem *> items; // Map of PID to tree items

    for (const QString &entry : entries)
    {
        bool isNumeric;
        int pid = entry.toInt(&isNumeric);
        if (!isNumeric)
        {
            continue; // Skip non-numeric entries
        }

        QString cmdline, status = "Unknown";
        double memory = 0.0, cpu = 0.0;
        int ppid = 0;

        // Read cmdline
        QFile cmdlineFile(QString("/proc/%1/cmdline").arg(pid));
        if (cmdlineFile.open(QIODevice::ReadOnly))
        {
            QByteArray cmdlineData = cmdlineFile.readAll();
            cmdline = QString(cmdlineData).replace('\0', ' ').trimmed();
            if (cmdline.isEmpty())
            {
                cmdline = QString("[PID %1]").arg(pid);
            }
        }

        // Read status and parent PID
        QFile statusFile(QString("/proc/%1/status").arg(pid));
        if (statusFile.open(QIODevice::ReadOnly))
        {
            QByteArray statusData = statusFile.readAll();
            QStringList lines = QString(statusData).split('\n');
            for (const QString &line : lines)
            {
                if (line.startsWith("State:"))
                {
                    QStringList parts = line.split(QRegExp("\\s+"));
                    if (parts.size() > 1)
                    {
                        status = parts[1]; // Extract status character
                        if (status == "R")
                        {
                            status = "Running";
                        }
                        else if (status == "S")
                        {
                            status = "Sleeping";
                        }
                        else if (status == "D")
                        {
                            status = "Disk Sleep";
                        }
                        else if (status == "T")
                        {
                            status = "Stopped";
                        }
                        else if (status == "Z")
                        {
                            status = "Zombie";
                        }
                        else
                        {
                            status = "Unknown";
                        }
                    }
                }
                else if (line.startsWith("VmRSS:"))
                {
                    QStringList parts = line.split(QRegExp("\\s+"));
                    if (parts.size() > 1)
                    {
                        memory = parts[1].toDouble() / 1024.0; // Convert to MiB
                    }
                }
                else if (line.startsWith("PPid:"))
                {
                    QStringList parts = line.split(QRegExp("\\s+"));
                    if (parts.size() > 1)
                    {
                        ppid = parts[1].toInt();
                    }
                }
            }
        }

        // Create a new tree item
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, cmdline);                                  // Process Name
        item->setText(1, status);                                   // Status
        item->setText(2, QString::number(cpu, 'f', 2) + "%");       // %CPU (placeholder)
        item->setText(3, QString::number(pid));                     // PID
        item->setText(4, QString::number(memory, 'f', 2) + " MiB"); // Memory

        // Set the item's icon
        item->setIcon(0, QIcon::fromTheme("application-x-executable")); // Example icon

        items[pid] = item;

        // Attach to parent if applicable
        if (items.contains(ppid))
        {
            items[ppid]->addChild(item);
        }
        else
        {
            processTree->addTopLevelItem(item);
        }
    }
}

void ProcessMonitor::showContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *selectedItem = processTree->itemAt(pos);
    if (!selectedItem)
    {
        return;
    }

    int pid = selectedItem->text(3).toInt();
    QString processName = selectedItem->text(0);

    QMenu contextMenu(this);
    QAction *stopAction = contextMenu.addAction("Stop Process");
    QAction *continueAction = contextMenu.addAction("Continue Process");
    QAction *killAction = contextMenu.addAction("Kill Process");
    QAction *memoryMapsAction = contextMenu.addAction("List Memory Maps");
    QAction *openFilesAction = contextMenu.addAction("List Open Files");

    QAction *selectedAction = contextMenu.exec(processTree->viewport()->mapToGlobal(pos));

    if (selectedAction == stopAction)
    {
        kill(pid, SIGSTOP);
    }
    else if (selectedAction == continueAction)
    {
        kill(pid, SIGCONT);
    }
    else if (selectedAction == killAction)
    {
        kill(pid, SIGKILL);
        refreshProcesses(); // Refresh after killing process
    }
    else if (selectedAction == memoryMapsAction)
    {
        showMemoryMapsDialog(processName, pid);
    }
    else if (selectedAction == openFilesAction)
    {
        showOpenFilesDialog(processName, pid);
    }
}

void ProcessMonitor::updateButtonState()
{
    endProcessButton->setEnabled(processTree->selectedItems().count() > 0);
}

void ProcessMonitor::endProcess()
{
    QTreeWidgetItem *selectedItem = processTree->currentItem();
    if (selectedItem)
    {
        int pid = selectedItem->text(3).toInt();
        kill(pid, SIGKILL);
        refreshProcesses();
    }
}

int ProcessMonitor::getSelectedPID()
{
    QModelIndexList selection = processTree->selectionModel()->selectedRows();
    if (selection.isEmpty())
    {
        QMessageBox::warning(this, "No Selection", "Please select a process.");
        return -1;
    }
    int row = selection.first().row();
    return processTree->topLevelItem(row)->text(3).toInt();
}

void ProcessMonitor::stopProcess()
{
    int pid = getSelectedPID();
    if (pid != -1)
    {
        if (kill(pid, SIGSTOP) == 0)
        {
            QMessageBox::information(this, "Process Stopped", QString("Process %1 stopped.").arg(pid));
        }
        else
        {
            QMessageBox::critical(this, "Error", "Failed to stop process.");
        }
    }
}

void ProcessMonitor::continueProcess()
{
    int pid = getSelectedPID();
    if (pid != -1)
    {
        if (kill(pid, SIGCONT) == 0)
        {
            QMessageBox::information(this, "Process Continued", QString("Process %1 continued.").arg(pid));
        }
        else
        {
            QMessageBox::critical(this, "Error", "Failed to continue process.");
        }
    }
}

void ProcessMonitor::killProcess()
{
    int pid = getSelectedPID();
    if (pid != -1)
    {
        if (kill(pid, SIGKILL) == 0)
        {
            QMessageBox::information(this, "Process Killed", QString("Process %1 killed.").arg(pid));
            refreshProcesses(); // Refresh after killing process
        }
        else
        {
            QMessageBox::critical(this, "Error", "Failed to kill process.");
        }
    }
}

void ProcessMonitor::showMemoryMaps()
{
    int pid = getSelectedPID();
    if (pid != -1)
    {
        QFile mapsFile(QString("/proc/%1/maps").arg(pid));
        if (mapsFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox::information(this, "Memory Maps", mapsFile.readAll());
        }
        else
        {
            QMessageBox::critical(this, "Error", "Failed to open memory maps.");
        }
    }
}

void ProcessMonitor::showOpenFiles()
{
    int pid = getSelectedPID();
    if (pid != -1)
    {
        QProcess process;
        process.start("lsof", QStringList() << "-p" << QString::number(pid));
        process.waitForFinished();
        QString output = process.readAllStandardOutput();
        QMessageBox::information(this, "Open Files", output.isEmpty() ? "No open files." : output);
    }
}

void ProcessMonitor::showProcessDetails()
{
    int pid = getSelectedPID();
    if (pid != -1)
    {
        QFile statusFile(QString("/proc/%1/status").arg(pid));
        if (statusFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox::information(this, "Process Details", statusFile.readAll());
        }
        else
        {
            QMessageBox::critical(this, "Error", "Failed to open process details.");
        }
    }
}

void ProcessMonitor::showOpenFilesDialog(QString processName, int pid)
{
    // Create the dialog
    QDialog *dialog = new QDialog();
    dialog->setWindowTitle(QString("Open Files"));
    dialog->resize(600, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

    // Add title label
    QLabel *titleLabel = new QLabel(QString("Files opened by process \"%1\" (PID %2)").arg(processName).arg(pid), dialog);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Add table widget
    QTableWidget *fileTable = new QTableWidget(dialog);
    fileTable->setColumnCount(3); // FD, Type, Object
    fileTable->setHorizontalHeaderLabels({"FD", "Type", "Object"});
    fileTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    fileTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make table read-only
    fileTable->setSelectionMode(QAbstractItemView::NoSelection);   // Disable selection
    mainLayout->addWidget(fileTable);

    // Add close button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *closeButton = new QPushButton("Close", dialog);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    mainLayout->addLayout(buttonLayout);

    // Load open files using lsof
    QProcess process;
    process.start("lsof", QStringList() << "-p" << QString::number(pid));
    process.waitForFinished();
    QString output = process.readAllStandardOutput();

    // Parse lsof output and populate the table
    QStringList lines = output.split('\n', QString::SkipEmptyParts);
    for (int i = 1; i < lines.size(); ++i)
    { // Skip the first line (header)
        QStringList columns = lines[i].split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if (columns.size() < 9)
        {
            continue; // Skip invalid lines
        }

        // Extract relevant columns (FD, TYPE, NAME)
        QString fdRaw = columns[3];
        QString type = columns[4];
        QString object = columns.mid(8).join(" "); // Combine remaining columns for the file path

        QString fd = fdRaw.remove(QRegExp("[^0-9]")); // Remove non-digit characters
        if (fd.isEmpty())
        {
            fd = columns[3];
        }

        int row = fileTable->rowCount();
        fileTable->insertRow(row);
        fileTable->setItem(row, 0, new QTableWidgetItem(fd));
        fileTable->setItem(row, 1, new QTableWidgetItem(type));
        fileTable->setItem(row, 2, new QTableWidgetItem(object));
    }

    // Connect close button
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);

    // Show the dialog
    dialog->exec();
}

void ProcessMonitor::showMemoryMapsDialog(QString processName, int pid)
{
    // Create the dialog
    QDialog *dialog = new QDialog();
    dialog->setWindowTitle(QString("Memory Maps"));
    dialog->resize(900, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

    // Add title label
    QLabel *titleLabel = new QLabel(QString("Memory maps for process \"%1\" (PID %2)").arg(processName).arg(pid), dialog);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Add table widget
    QTableWidget *memoryTable = new QTableWidget(dialog);
    memoryTable->setColumnCount(9); // Columns as described
    memoryTable->setHorizontalHeaderLabels({"Filename", "VM Start", "VM End", "VM Size", "Flags", "VM Offset",
                                            "Private clean", "Private dirty", "Shared clean", "Shared dirty"});
    memoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    memoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make table read-only
    memoryTable->setSelectionMode(QAbstractItemView::NoSelection);   // Disable selection
    mainLayout->addWidget(memoryTable);

    // Add close button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *closeButton = new QPushButton("Close", dialog);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    mainLayout->addLayout(buttonLayout);

    // Load memory maps from /proc/[pid]/smaps
    QFile mapsFile(QString("/proc/%1/smaps").arg(pid));
    if (!mapsFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(dialog, "Error", "Failed to retrieve memory maps. Ensure the application has appropriate permissions.");
        dialog->exec();
        return;
    }

    QByteArray fileContent = mapsFile.readAll();
    mapsFile.close();

    QStringList lines = QString(fileContent).split('\n', QString::SkipEmptyParts);

    QString filename = "";
    QString vmStart, vmEnd, flags, vmOffset;
    double vmSize = 0.0, privateClean = 0.0, privateDirty = 0.0, sharedClean = 0.0, sharedDirty = 0.0;

    for (const QString &line : lines)
    {
        if (line.contains('-'))
        { // Memory range line
            if (!filename.isEmpty())
            {
                // Add the previous entry to the table
                int row = memoryTable->rowCount();
                memoryTable->insertRow(row);
                memoryTable->setItem(row, 0, new QTableWidgetItem(filename));
                memoryTable->setItem(row, 1, new QTableWidgetItem(vmStart));
                memoryTable->setItem(row, 2, new QTableWidgetItem(vmEnd));
                memoryTable->setItem(row, 3, new QTableWidgetItem(QString("%1 KiB").arg(vmSize, 0, 'f', 2)));
                memoryTable->setItem(row, 4, new QTableWidgetItem(flags));
                memoryTable->setItem(row, 5, new QTableWidgetItem(vmOffset));
                memoryTable->setItem(row, 6, new QTableWidgetItem(QString("%1 bytes").arg(privateClean, 0, 'f', 2)));
                memoryTable->setItem(row, 7, new QTableWidgetItem(QString("%1 bytes").arg(privateDirty, 0, 'f', 2)));
                memoryTable->setItem(row, 8, new QTableWidgetItem(QString("%1 bytes").arg(sharedClean, 0, 'f', 2)));
                memoryTable->setItem(row, 9, new QTableWidgetItem(QString("%1 bytes").arg(sharedDirty, 0, 'f', 2)));
            }

            // Parse the new memory range
            QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if (parts.size() < 5)
                continue;

            vmStart = parts[0].split('-')[0];
            vmEnd = parts[0].split('-')[1];
            flags = parts[1];
            vmOffset = parts[2];
            filename = (parts.size() > 5) ? parts[5] : "[Anonymous]";

            // Reset memory details for the new entry
            vmSize = 0.0;
            privateClean = 0.0;
            privateDirty = 0.0;
            sharedClean = 0.0;
            sharedDirty = 0.0;
        }
        else if (line.startsWith("Size:"))
        {
            vmSize = line.split(QRegExp("\\s+"))[1].toDouble();
        }
        else if (line.startsWith("Private_Clean:"))
        {
            privateClean = line.split(QRegExp("\\s+"))[1].toDouble();
        }
        else if (line.startsWith("Private_Dirty:"))
        {
            privateDirty = line.split(QRegExp("\\s+"))[1].toDouble();
        }
        else if (line.startsWith("Shared_Clean:"))
        {
            sharedClean = line.split(QRegExp("\\s+"))[1].toDouble();
        }
        else if (line.startsWith("Shared_Dirty:"))
        {
            sharedDirty = line.split(QRegExp("\\s+"))[1].toDouble();
        }
    }

    // Add the last entry
    if (!filename.isEmpty())
    {
        int row = memoryTable->rowCount();
        memoryTable->insertRow(row);
        memoryTable->setItem(row, 0, new QTableWidgetItem(filename));
        memoryTable->setItem(row, 1, new QTableWidgetItem(vmStart));
        memoryTable->setItem(row, 2, new QTableWidgetItem(vmEnd));
        memoryTable->setItem(row, 3, new QTableWidgetItem(QString("%1 KiB").arg(vmSize, 0, 'f', 2)));
        memoryTable->setItem(row, 4, new QTableWidgetItem(flags));
        memoryTable->setItem(row, 5, new QTableWidgetItem(vmOffset));
        memoryTable->setItem(row, 6, new QTableWidgetItem(QString("%1 bytes").arg(privateClean, 0, 'f', 2)));
        memoryTable->setItem(row, 7, new QTableWidgetItem(QString("%1 bytes").arg(privateDirty, 0, 'f', 2)));
        memoryTable->setItem(row, 8, new QTableWidgetItem(QString("%1 bytes").arg(sharedClean, 0, 'f', 2)));
        memoryTable->setItem(row, 9, new QTableWidgetItem(QString("%1 bytes").arg(sharedDirty, 0, 'f', 2)));
    }

    // Connect close button
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);

    // Show the dialog
    dialog->exec();
}
