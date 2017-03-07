#ifndef LOGMONITOR_H
#define LOGMONITOR_H

#define FAILED 0
#define ACCEPT 1
#define DISCON 2

#include <QObject>
#include <QDateTime>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
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
    void readActivityLog();
    void saveActivityLog();
    void updateActivityLog(const QString &str, int type);

private:

    int allowedAttempts;
    int attemptResetHr;
    int attemptResetMin;
    int blockHr;
    int blockMin;

    QFile activityLog;
    QJsonDocument activityLogJson;

    QFileInfo oldFileInfo;
    QFile logFile;

    QTextStream *ts;

    QRegularExpression sshRe;
    QRegularExpression ipRe;
};

#endif // LOGMONITOR_H
