#ifndef LOGMONITOR_H
#define LOGMONITOR_H

#include <QObject>
#include <QString>
#include <QDebug>

class LogMonitor : public QObject
{
    Q_OBJECT

public:
    LogMonitor(QString logFile, QString attempts, QString resetHrs, QString resetMins, QString blockHr, QString blockMin);
    ~LogMonitor();

    QString file;

public slots:

    void handleChange(const QString &str);

private:

    int allowedAttempts;
    int attemptResetHr;
    int attemptResetMin;
    int blockHr;
    int blockMin;
};

#endif // LOGMONITOR_H
