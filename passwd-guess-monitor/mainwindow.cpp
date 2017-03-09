#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initUi();
    currentDir = QCoreApplication::applicationDirPath();
    createActivityLog();
    attachToDaemon();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (monitorer->running)
    {
        monitorer->stopWork();
    }

    if (running)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Stop Daemon?", "Would you like to kill the daemon?", QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            if (fileExists(LOG_MONITOR_LOGFILE))
            {
                QFile f(LOG_MONITOR_LOGFILE);
                if (f.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    int oldPid = f.readAll().toInt();
                    QProcess::execute(QString("kill " + QString::number(oldPid)));
                }
                f.close();
            }
        }
    }
}

void MainWindow::initUi()
{
    ui->infoTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList logFiles;

    logFiles << "/var/log/auth.log";    // ubuntu
    logFiles << "/var/log/secure";      // fedora

    ui->logFile->addItems(logFiles);
}

void MainWindow::attachToDaemon()
{
    QString path = currentDir + "/log-monitor.settings";

    // if there is a settings file, then the daemon is running
    if (fileExists(path))
    {
        running = true;

        // get current settings, update UI
        QFile settingsFile("log-monitor.settings");
        if (!settingsFile.open(QIODevice::ReadOnly))
        {
            qDebug() << "failed to open settings file.";
            return;
        }

        QTextStream stream(&settingsFile);
        QJsonDocument doc = QJsonDocument::fromJson(stream.readAll().toLocal8Bit());

        settingsFile.close();

        file            = doc.object()["log_file"].isString();
        allowedAttempts = doc.object()["allowed_attempts"].toInt();
        resetHr         = doc.object()["attempt_reset_time_hr"].toInt();
        resetMin        = doc.object()["attempt_reset_time_min"].toInt();
        blockHr         = doc.object()["block_time_hr"].toInt();
        blockMin        = doc.object()["block_time_min"].toInt();

        file == "/var/log/auth.log" ? ui->logFile->setCurrentIndex(1) : ui->logFile->setCurrentIndex(0);
        ui->permittedAttempts->setValue(allowedAttempts);
        ui->resetHrs->setValue(resetHr);
        ui->resetMins->setValue(resetMin);
        ui->blockHrs->setValue(blockHr);
        ui->blockMins->setValue(blockMin);

        toggleUi();

        // get the current login attempt activity, update table
        QFile activityFile(currentDir + "/activity.log");

        if (!activityFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "failed to open activity file.";
            return;
        }

        QTextStream ts(&activityFile);
        updateTable(ts.readAll());
        activityFile.close();
    }
}

void MainWindow::toggleUi()
{
    ui->logFile->setEnabled(!running);
    ui->permittedAttempts->setEnabled(!running);
    ui->resetHrs->setEnabled(!running);
    ui->resetMins->setEnabled(!running);
    ui->blockHrs->setEnabled(!running);
    ui->blockMins->setEnabled(!running);

    if (running)
    {
        ui->startStopBtn->setText("Stop Log Monitor Daemon");
        ui->startStopBtn->setStyleSheet("QPushButton { background-color: rgb(204, 0, 0); color: white;}");
        ui->daemonStatus->setText("ACTIVE");
        ui->daemonStatus->setStyleSheet("QLabel {color : green; }");
    }
    else
    {
        ui->startStopBtn->setText("Start Log Monitor Daemon");
        ui->startStopBtn->setStyleSheet("QPushButton { background-color: rgb(78, 154, 6); color: black;}");
        ui->daemonStatus->setText("INACTIVE");
        ui->daemonStatus->setStyleSheet("QLabel {color : red; }");
    }
}

void MainWindow::on_startStopBtn_clicked()
{
    if (running)
    {
        stop();
        return;
    }
    start();
}

void MainWindow::on_unblockBtn_clicked()
{
    // empty the activity log
    QString path = currentDir + "/activity.log";

    QFile activityLog(path);
    if (!activityLog.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "failed to open activity log file.";
        return;
    }

    QJsonDocument jd = QJsonDocument();  // sets to null

    activityLog.write(jd.toJson(QJsonDocument::Indented));
    activityLog.close();

    // execute iptable command to remove user chain for ip blocks
    QProcess::execute("iptables -F ip_block");
}

void MainWindow::start()
{
    QProcess *logMonitor = new QProcess();

    QString logFile     = ui->logFile->currentText();
    QString attempts    = ui->permittedAttempts->text() + " ";
    QString resetHrs    = ui->resetHrs->text().split(" ")[0] + " ";
    QString resetMins   = ui->resetMins->text().split(" ")[0] + " ";
    QString blockHr     = ui->blockHrs->text().split(" ")[0] + " ";
    QString blockMin    = ui->blockMins->text().split(" ")[0] + " ";

    if (!fileExists(logFile))
    {
        QMessageBox::information(this, "Error", QString("Log file \"" + logFile + "\" not found!"));
        return;
    }

    QString args = logFile + " " + attempts + resetHrs + resetMins + blockHr + blockMin;

    QString command = currentDir + "/log-monitor " + args;

    startMonitorer();

    logMonitor->startDetached(command);

    delete logMonitor;

    running = true;

    toggleUi();
}

void MainWindow::stop()
{
    if (fileExists(LOG_MONITOR_LOGFILE))
    {
        QFile f(LOG_MONITOR_LOGFILE);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            int oldPid = f.readAll().toInt();
            QProcess::execute(QString("kill " + QString::number(oldPid)));
        }
        f.close();
    }

    ui->infoTable->clearContents();
    ui->infoTable->setRowCount(0);
    running = false;
    toggleUi();
}

// TODO: update table on progam launch if activitly log present
void MainWindow::updateTable(QString data)
{
    jsonDoc = QJsonDocument::fromJson(data.toLocal8Bit());

    if (jsonDoc.isEmpty() || jsonDoc.isNull())
    {
        ui->infoTable->clearContents();
        ui->infoTable->setRowCount(0);
        return;
    }

    QJsonObject root = jsonDoc.object();
    QJsonArray array = root.value("login_attempts").toArray();

    QString ip;
    QString status;
    QString program;
    QString attempts;
    QString formattedTime;
    QDateTime time;
    qint64 t;

    ui->infoTable->setRowCount(array.size());

    for (int i = 0; i < array.size(); i++)
    {
        QJsonObject obj = array.at(i).toObject();

        ip          = obj["ip"].toString();
        status      = obj["status"].toString();
        program     = obj["program"].toString();
        attempts    = QString::number(obj["attempts"].toInt());
        t           = obj["time"].toString().toLongLong();

        time.setSecsSinceEpoch(t);
        formattedTime = time.toString("hh:mm:ss ap");

        QColor c;
        if (status == "Failed Attempt")
        {
            c = QColor::fromRgb(237, 135, 135); // light red
        }
        else if (status == "Connected")
        {
            c = QColor::fromRgb(86, 232, 41);   // green
        }
        else if (status == "Disconnected")
        {
            c = QColor::fromRgb(212, 255, 163); // light green
        }
        else if (status == "Attempts Reset")
        {
            c = QColor::fromRgb(255, 250, 124); // light yellow
        }
        else if (status == "IP Blocked")
        {
            c = QColor::fromRgb(255, 7, 7);     // red
        }
        else if (status == "IP Block Expired")
        {
            c = QColor::fromRgb(255, 184, 33);   // orange
        }

        ui->infoTable->setItem(i, 0, new QTableWidgetItem(ip));
        ui->infoTable->setItem(i, 1, new QTableWidgetItem(status));
        ui->infoTable->setItem(i, 2, new QTableWidgetItem(program));
        ui->infoTable->setItem(i, 3, new QTableWidgetItem(attempts));
        ui->infoTable->setItem(i, 4, new QTableWidgetItem(formattedTime));

        // set row color
        for (int j = 0; j < 5; j++)
        {
            ui->infoTable->item(i, j)->setBackgroundColor(c);
        }
    }
}

void MainWindow::startMonitorer()
{
    QThread *monitorThread;

    monitorThread   = new QThread;
    monitorer       = new ActivityLogMonitor(currentDir + "/activity.log");

    monitorer->moveToThread(monitorThread);

    connect(monitorer, SIGNAL(finished()), monitorThread, SLOT(quit()));
    connect(monitorer, SIGNAL(finished()), monitorer, SLOT(deleteLater()));
    connect(monitorThread, SIGNAL(finished()), monitorThread, SLOT(deleteLater()));

    connect(monitorer, SIGNAL(updatedAttempts(QString)), this, SLOT(updateTable(QString)));

    monitorThread->start();
}

void MainWindow::createActivityLog()
{
    QString path = currentDir + "/activity.log";
    if (!fileExists(path))
    {
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "open activity.log failure";
        }
        f.close();
    }
}

bool MainWindow::fileExists(QString path)
{
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}

void MainWindow::on_resetHrs_valueChanged(int arg1)
{
    resetHr = arg1;
    if (arg1 == 24)
    {
        ui->resetMins->setValue(0);
        resetMin = 0;
    }
    else if (arg1 == 0 && resetMin == 0)
    {
        ui->resetMins->setValue(1);
        resetMin = 1;
    }
}

void MainWindow::on_resetMins_valueChanged(int arg1)
{
    resetMin = arg1;

    if (arg1 == 60)
    {
        resetHr += 1;
        ui->resetHrs->setValue(resetHr);
        ui->resetMins->setValue(0);
        resetMin = 0;
    }
    else if (arg1 == 0 && resetHr == 0)
    {
        ui->resetMins->setValue(1);
        resetMin = 1;
    }
    else if (resetHr == 24)
    {
        ui->resetMins->setValue(0);
        resetMin = 0;
    }
}

void MainWindow::on_blockHrs_valueChanged(int arg1)
{
    blockHr = arg1;
    if (arg1 == 24)
    {
        ui->blockMins->setValue(0);
        blockMin = 0;
    }
    else if (arg1 == 0 && blockMin == 0)
    {
        ui->blockMins->setValue(1);
        blockMin = 1;
    }
}

void MainWindow::on_blockMins_valueChanged(int arg1)
{
    blockMin = arg1;

    if (arg1 == 60)
    {
        blockHr += 1;
        ui->blockHrs->setValue(blockHr);
        ui->blockMins->setValue(0);
        blockMin = 0;
    }
    else if (arg1 == 0 && blockHr == 0)
    {
        ui->blockMins->setValue(1);
        blockMin = 1;
    }
    else if (blockHr == 24)
    {
        ui->blockMins->setValue(0);
        blockMin = 0;
    }
}
