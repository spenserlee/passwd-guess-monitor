#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initUi();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{

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

    QString currentDir(QDir::currentPath());

    QString logFile     = ui->logFile->currentText() + " ";
    QString attempts    = ui->permittedAttempts->text() + " ";
    QString resetHrs    = ui->resetHrs->text().split(" ")[0] + " ";
    QString resetMins   = ui->resetMins->text().split(" ")[0] + " ";
    QString blockHr     = ui->blockHrs->text().split(" ")[0] + " ";
    QString blockMin    = ui->blockMins->text().split(" ")[0] + " ";

    QString args = logFile + attempts + resetHrs + resetMins + blockHr + blockMin;

    QString command = currentDir + "/log-monitor " + args;

    qDebug() << command;

    logMonitor->startDetached(command);

    delete logMonitor;
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
