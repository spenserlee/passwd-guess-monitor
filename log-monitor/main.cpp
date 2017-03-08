#include <QCoreApplication>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QTextStream>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

#include <csignal>

#include "logmonitor.h"

void signalHandler(int signum);
bool fileExists(QString path);
void closeOtherDaemons();

LogMonitor *lm;

int main(int argc, char *argv[])
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGHUP, signalHandler);  // received when terminal is closed

    QCoreApplication a(argc, argv);

    closeOtherDaemons();

    QStringList args = a.arguments();

    if (args.size() < 6)
    {
        qDebug() << "missing arguments... exiting.";
        return -1;
    }

    QFileSystemWatcher watcher;

    if (!watcher.addPath(args.at(1)))
    {
        qDebug() << "failed to add watch to " << args.at(1) << ". exiting.";
        QFile pidFile(PID_FILE);
        pidFile.remove();   // sometimes fails to actually remove file
        return -2;
    }

    // TODO: consider implementing full cmd options
    // https://doc.qt.io/qt-5/qcommandlineparser.html#details
    lm = new LogMonitor(args.at(1), args.at(2), args.at(3), args.at(4), args.at(5), args.at(6));

    QObject::connect(&watcher, SIGNAL(fileChanged(QString)), lm, SLOT(handleChange(QString)));

    return a.exec();
}

void signalHandler(int signum)
{
    QFile pidFile(PID_FILE);
    QFile settingsFile(SETTINGS_FILE);
    pidFile.remove();
    settingsFile.remove();
    lm->emptyActivityLog();
    exit(signum);
}

bool fileExists(QString path)
{
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}

// TODO: consider switching to using a mutex rather than a pid file
void closeOtherDaemons()
{
    QFile f(PID_FILE);

    if (fileExists(PID_FILE))
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
        f.close();
    }
}
