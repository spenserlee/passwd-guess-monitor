#ifndef LOGMONITOR_H
#define LOGMONITOR_H

#include <QObject>
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

class LogMonitor : public QObject
{
    Q_OBJECT

public:
    LogMonitor(QString path, QString attempts, QString resetHrs, QString resetMins, QString blockHr, QString blockMin);
    ~LogMonitor();

    QString file;

public slots:

    void handleChange(const QString &path);

private slots:
    void parseChanges(const QString &str);

private:

    int allowedAttempts;
    int attemptResetHr;
    int attemptResetMin;
    int blockHr;
    int blockMin;

    QFileInfo oldFileInfo;
    QFile logFile;

    QTextStream *ts;
};

#endif // LOGMONITOR_H
