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

//    qDebug() << "file = " << file;
//    qDebug() << "allowedAttempts = " << allowedAttempts;
//    qDebug() << "attemptResetHr  = " << attemptResetHr;
//    qDebug() << "attemptResetMin = " << attemptResetMin;
//    qDebug() << "blockHr   = " << this->blockHr;
//    qDebug() << "blockMin  = " << this->blockMin;
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
    qDebug() << str;
    /*
      if contains sshd
        if contains Failed password or Accepted password
            get time
            get IP
            updateActivityLog()
    */
}
