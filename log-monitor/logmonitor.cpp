#include "logmonitor.h"

LogMonitor::LogMonitor(QString logFile, QString attempts, QString resetHrs, QString resetMins, QString blockHr, QString blockMin)
{
    file = logFile;

    allowedAttempts = attempts.toInt();
    attemptResetHr  = resetHrs.toInt();
    attemptResetMin = resetMins.toInt();
    this->blockHr   = blockHr.toInt();
    this->blockMin  = blockMin.toInt();

    qDebug() << "file = " << file;
    qDebug() << "allowedAttempts = " << allowedAttempts;
    qDebug() << "attemptResetHr  = " << attemptResetHr;
    qDebug() << "attemptResetMin = " << attemptResetMin;
    qDebug() << "blockHr   = " << this->blockHr;
    qDebug() << "blockMin  = " << this->blockMin;
}

LogMonitor::~LogMonitor()
{

}

void LogMonitor::handleChange(const QString &str)
{
    qDebug() << str;
}
