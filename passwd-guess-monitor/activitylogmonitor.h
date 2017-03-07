#ifndef ACTIVITYLOGMONITOR_H
#define ACTIVITYLOGMONITOR_H

#include <QDebug>
#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

class ActivityLogMonitor : public QObject
{
    Q_OBJECT
public:
    explicit ActivityLogMonitor(QObject *parent = 0);
    ActivityLogMonitor(QString file);
    bool running;

public slots:
    void handleChange(const QString &path);
    void stopWork();

signals:
    void updatedAttempts(QString data);
    void finished();

private:
    QString filePath;
    QFile logFile;

};

#endif // ACTIVITYLOGMONITOR_H
