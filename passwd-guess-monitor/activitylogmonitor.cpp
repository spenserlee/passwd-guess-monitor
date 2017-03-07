#include "activitylogmonitor.h"

ActivityLogMonitor::ActivityLogMonitor(QObject *parent) : QObject(parent)
{

}

ActivityLogMonitor::ActivityLogMonitor(QString file)
{
    filePath = file;
    running = true;

    logFile.setFileName(filePath);
}

void ActivityLogMonitor::handleChange(const QString &path)
{
    if (path == filePath) // the activity.log file was changed
    {

        if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "failed to open log file.";
            return;
        }

        QTextStream ts(&logFile);
        QString data = ts.readAll();

        emit updatedAttempts(data);

        logFile.close();
    }
}

void ActivityLogMonitor::stopWork()
{
    running = false;
    emit finished();
}
