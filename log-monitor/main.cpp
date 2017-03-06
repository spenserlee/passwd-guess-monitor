#include <QCoreApplication>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QTextStream>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

#include <csignal>

#include "logmonitor.h"

#define LOG_MONITOR_LOGFILE "log-monitor.log"

// TODO: consider implementing full cmd options
// https://doc.qt.io/qt-5/qcommandlineparser.html#details
//#include <QCommandLineParser>
//#include <QCommandLineOption>

void signalHandler(int signum)
{
    QFile f(LOG_MONITOR_LOGFILE);
    f.remove();
    exit(signum);
}

bool fileExists(QString path)
{
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}

void closeOtherDaemons()
{
    QFile f(LOG_MONITOR_LOGFILE);

    if (fileExists(LOG_MONITOR_LOGFILE))
    {
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            int oldPid = f.readAll().toInt();
            QProcess::execute(QString("kill " + QString::number(oldPid)));
        }
        f.close();
    }

    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QTextStream ts(&f);
        ts << QCoreApplication::applicationPid();
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    QCoreApplication a(argc, argv);

    closeOtherDaemons();

    QStringList args = a.arguments();

    if (args.size() < 6)
    {
        qDebug() << "missing arguments... exiting.";
        return -1;
    }

    LogMonitor *lm = new LogMonitor(args.at(1), args.at(2), args.at(3), args.at(4), args.at(5), args.at(6));

    QFileSystemWatcher watcher;

    if (!watcher.addPath(lm->file))
    {
        qDebug() << "failed to add watch to " << lm->file << ". exiting.";
        return -2;
    }

    // consider using directoryChanged() over fileChanged()
    // fileChanged() can fail if program editing log deletes and replaces file with modifications

    QObject::connect(&watcher, SIGNAL(fileChanged(QString)), lm, SLOT(handleChange(QString)));

    return a.exec();
}
