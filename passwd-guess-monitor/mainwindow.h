#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void initUi();

    void on_unblockBtn_clicked();

    void on_registerBtn_clicked();

    void on_resetHrs_valueChanged(int arg1);

    void on_resetMins_valueChanged(int arg1);

    void on_blockHrs_valueChanged(int arg1);

    void on_blockMins_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;

    int resetHr = 0;
    int resetMin = 0;
    int blockHr = 0;
    int blockMin = 0;

};

#endif // MAINWINDOW_H
