#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initUi();
    currentDir = QDir::currentPath();
    createActivityLog();
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
}

void MainWindow::initUi()
{
    ui->infoTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList logFiles;

    logFiles << "/var/log/auth.log";    // ubuntu
    logFiles << "/var/log/secure";      // fedora

    ui->logFile->addItems(logFiles);
}

void MainWindow::on_unblockBtn_clicked()
{
    // execute iptable command to remove user chain for ip blocks
}

void MainWindow::on_startBtn_clicked()
{
    QProcess *logMonitor = new QProcess();

    QString logFile     = ui->logFile->currentText() + " ";
    QString attempts    = ui->permittedAttempts->text() + " ";
    QString resetHrs    = ui->resetHrs->text().split(" ")[0] + " ";
    QString resetMins   = ui->resetMins->text().split(" ")[0] + " ";
    QString blockHr     = ui->blockHrs->text().split(" ")[0] + " ";
    QString blockMin    = ui->blockMins->text().split(" ")[0] + " ";

    QString args = logFile + attempts + resetHrs + resetMins + blockHr + blockMin;

    QString command = currentDir + "/log-monitor " + args;

    startMonitorer();

    logMonitor->startDetached(command);

    delete logMonitor;
}

// TODO: update table on progam launch if activitly log present
void MainWindow::updateTable(QString data)
{
    jsonDoc = QJsonDocument::fromJson(data.toLocal8Bit());

    if (jsonDoc.isNull())
    {
        ui->infoTable->clear();
        return;
    }

    QJsonObject root = jsonDoc.object();
    QJsonArray array = root.value("login_attempts").toArray();

    QString ip;
    QString status;
    QString program;
    QString attempts;
    QString lastAttempt;

    ui->infoTable->setRowCount(array.size());

    for (int i = 0; i < array.size(); i++)
    {
        QJsonObject obj = array.at(i).toObject();

        ip          = obj["ip"].toString();
        status      = obj["status"].toString();
        program     = obj["program"].toString();
        attempts    = QString::number(obj["attempts"].toInt());
        lastAttempt = obj["time"].toString();

        ui->infoTable->setItem(i, 0, new QTableWidgetItem(ip));
        ui->infoTable->setItem(i, 1, new QTableWidgetItem(status));
        ui->infoTable->setItem(i, 2, new QTableWidgetItem(program));
        ui->infoTable->setItem(i, 3, new QTableWidgetItem(attempts));
        ui->infoTable->setItem(i, 4, new QTableWidgetItem(lastAttempt));
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

void MainWindow::on_stopBtn_clicked()
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
    if (monitorer->running)
    {
        monitorer->stopWork();
    }
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
