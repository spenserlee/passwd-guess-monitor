#include <QCoreApplication>
#include <QFileSystemWatcher>
#include <QDebug>

#include "logmonitor.h"

// TODO: consider implementing full cmd options
// https://doc.qt.io/qt-5/qcommandlineparser.html#details
//#include <QCommandLineParser>
//#include <QCommandLineOption>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // TODO: check if an instance of log-monitor is already running

    QStringList args = a.arguments();

    if (args.size() < 6)
    {
        qDebug() << "missing arguments... exiting.";
        return -1;
    }

    LogMonitor *lm = new LogMonitor(args.at(1), args.at(2), args.at(3), args.at(4), args.at(5), args.at(6));

    QFileSystemWatcher watcher;

    watcher.addPath(lm->file);

    // consider using directoryChanged() over fileChanged()
    // fileChanged() can fail if program editing log deletes and replaces file with modifications

    QObject::connect(&watcher, SIGNAL(fileChanged(QString)), lm, SLOT(handleChange(QString)));

    return a.exec();  // only necessary if using an event loop
}
