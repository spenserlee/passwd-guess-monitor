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

void MainWindow::initUi()
{
    ui->infoTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList logFiles;

    logFiles << "/var/log/auth.log";
    logFiles << "/var/log/secure";

    ui->logFile->addItems(logFiles);


}

void MainWindow::on_unblockBtn_clicked()
{
    // execute iptable command to remove user chain for ip blocks
}

void MainWindow::on_registerBtn_clicked()
{
    // thread to monitor file in /opt/passwd-guess-monitor/activity.log

    // get user settings

    // execute script

    // add script to crontab

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
