#ifndef LOGMONITOR_H
#define LOGMONITOR_H

#define ATTEMPTS_FILE "activity.log"
#define SETTINGS_FILE "log-monitor.settings"
#define PID_FILE "log-monitor.pid"

#define FAILED 0
#define ACCEPT 1
#define DISCON 2

#include <QCoreApplication>
#include <QObject>
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

#include "ipblockmonitor.h"

class LogMonitor : public QObject
{
    Q_OBJECT

public:
    LogMonitor(QString path, QString attempts, QString resetHrs, QString resetMins, QString blockHr, QString blockMin);
    ~LogMonitor();


public slots:

    void handleChange(const QString &path);
    void emptyActivityLog();

private slots:
    void saveSettings();
    void readActivityLog();
    void checkForUnbanState();
    void saveActivityLog();
    void startBlockMonitor();
    void setIptablesUserChain();
    void parseChanges(const QString &str);
    void updateActivityLog(const QString &str, int type);

private:
    IpBlockMonitor *ipBlockMonitor;

    QString file;

    int allowedAttempts;
    int attemptResetHr;
    int attemptResetMin;
    int blockHr;
    int blockMin;

    QString currentPath;

    QFile activityLog;
    QJsonDocument activityLogJson;

    QFileInfo oldFileInfo;
    QFile logFile;

    QTextStream *ts;

    QRegularExpression sshRe;
    QRegularExpression ipRe;
};

#endif // LOGMONITOR_H
