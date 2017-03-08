#ifndef IPBLOCKMONITOR_H
#define IPBLOCKMONITOR_H

#include <QObject>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

#include <unistd.h>

class IpBlockMonitor : public QObject
{
    Q_OBJECT
public:
    explicit IpBlockMonitor(QObject *parent = 0);

    IpBlockMonitor(QJsonDocument *jsonDoc, int attempts, int attemptResetHr, int attemptResetMin, int blockHr, int blockMin);

signals:
    void saveActivityLog();
    void finished();

public slots:
    void monitor();
    void stopMonitoring();

private slots:

private:
    bool running;

    QJsonDocument *jsonDoc;

    int attempts;
    int attemptResetTime;
    int blockTime;
};

#endif // IPBLOCKMONITOR_H
