#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define LOG_MONITOR_LOGFILE "log-monitor.log"

#include <QMainWindow>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QDebug>

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

private slots:

    void initUi();

    void on_unblockBtn_clicked();

    void on_startBtn_clicked();

    void on_resetHrs_valueChanged(int arg1);

    void on_resetMins_valueChanged(int arg1);

    void on_blockHrs_valueChanged(int arg1);

    void on_blockMins_valueChanged(int arg1);

    bool fileExists(QString path);

    void on_stopBtn_clicked();

private:
    Ui::MainWindow *ui;

    int resetHr = 0;
    int resetMin = 0;
    int blockHr = 0;
    int blockMin = 0;

};

#endif // MAINWINDOW_H
