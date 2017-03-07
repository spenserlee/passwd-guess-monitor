#include "mainwindow.h"
#include <QApplication>
#include <QFileSystemWatcher>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    w.startMonitorer();  // create activity log monitor thread

    QFileSystemWatcher watcher;
    QString filePath = w.currentDir + "/activity.log";

    if (!watcher.addPath(filePath))
    {
        qDebug() << "failed to add watch to " << filePath << ". exiting.";
    }

    QObject::connect(&watcher, SIGNAL(fileChanged(QString)), w.monitorer, SLOT(handleChange(QString)));

    return a.exec();
}
