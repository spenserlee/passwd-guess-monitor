#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define LOG_MONITOR_LOGFILE "log-monitor.pid"

#include <QMainWindow>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QThread>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

#include <unistd.h>

#include "activitylogmonitor.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void closeEvent(QCloseEvent *event);
    ~MainWindow();

    void startMonitorer();
    ActivityLogMonitor *monitorer;
    QString currentDir;

private slots:

    void initUi();

    void attachToDaemon();

    void toggleUi();

    void on_startStopBtn_clicked();

    void start();

    void stop();

    void on_unblockBtn_clicked();

    void updateTable(QString data);

    void on_resetHrs_valueChanged(int arg1);

    void on_resetMins_valueChanged(int arg1);

    void on_blockHrs_valueChanged(int arg1);

    void on_blockMins_valueChanged(int arg1);

    void createActivityLog();

    bool fileExists(QString path);

private:
    Ui::MainWindow *ui;

    QFileSystemWatcher *watcher;

    QJsonDocument jsonDoc;

    bool running = false;

    QString file;
    int allowedAttempts = 0;
    int resetHr = 0;
    int resetMin = 0;
    int blockHr = 0;
    int blockMin = 0;

};

#endif // MAINWINDOW_H
