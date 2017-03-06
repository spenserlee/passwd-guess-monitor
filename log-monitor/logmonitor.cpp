#include "logmonitor.h"

LogMonitor::LogMonitor(QString path, QString attempts, QString resetHrs, QString resetMins, QString blockHr, QString blockMin)
{
    file = path;

    allowedAttempts = attempts.toInt();
    attemptResetHr  = resetHrs.toInt();
    attemptResetMin = resetMins.toInt();
    this->blockHr   = blockHr.toInt();
    this->blockMin  = blockMin.toInt();

    oldFileInfo = QFileInfo(file);
    logFile.setFileName(path);

    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "failed to open log file.";
        return;
    }

    ts = new QTextStream(&logFile);

    sshRe = QRegularExpression("sshd.+(?:[\r\n]|$)");
    ipRe = QRegularExpression("(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])");
}

LogMonitor::~LogMonitor()
{
    delete ts;
}

void LogMonitor::handleChange(const QString &path)
{
    QFileInfo newFileInfo(path);

    if (newFileInfo.size() != oldFileInfo.size())   // file has changed
    {
        int bytesToRead = newFileInfo.size() - oldFileInfo.size();

        if (!logFile.seek(logFile.size() - bytesToRead))
        {
            qDebug() << "failed to seek into log file";
            return;
        }

        QString buf = ts->read(bytesToRead);

        parseChanges(buf);
    }

    oldFileInfo = newFileInfo;
}

void LogMonitor::parseChanges(const QString &str)
{
    QRegularExpressionMatchIterator i = sshRe.globalMatch(str);

    QRegularExpressionMatch match;

    while (i.hasNext())
    {
        match = i.next();

        if (match.hasMatch())
        {
            QString matchString = match.captured(0);

            if (matchString.contains("Failed password"))
            {
                updateActivityLog(matchString, FAILED);
            }
            else if (matchString.contains("Accepted password"))
            {
                updateActivityLog(matchString, ACCEPT);
            }
            else if (matchString.contains("Disconnected from"))
            {
                updateActivityLog(matchString, DISCON);
            }
        }
    }
}

void LogMonitor::updateActivityLog(const QString &str, int type)
{
    QRegularExpressionMatch match = ipRe.match(str);
    QString status;

    switch(type)
    {
    case FAILED:
        status = "Failed Attempt";
        break;
    case ACCEPT:
        status = "Connected";
        break;
    case DISCON:
        status = "Disconnected";
        break;
    }

    if (match.hasMatch())
    {
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss ap");
        QString ip = match.captured(0);


        qDebug() << ip;
        qDebug() << status;
        qDebug() << time;
    }
}
