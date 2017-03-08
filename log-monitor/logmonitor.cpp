#include "logmonitor.h"

LogMonitor::LogMonitor(QString path, QString attempts, QString resetHrs, QString resetMins, QString blockHr, QString blockMin)
{
    file = path;

    allowedAttempts = attempts.toInt();
    attemptResetHr  = resetHrs.toInt();
    attemptResetMin = resetMins.toInt();
    this->blockHr   = blockHr.toInt();
    this->blockMin  = blockMin.toInt();

    saveSettings();
    readActivityLog();

    oldFileInfo = QFileInfo(file);
    logFile.setFileName(path);

    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "failed to open log file.";
        return;
    }

    ts = new QTextStream(&logFile);

    sshRe = QRegularExpression("sshd.+(?:[\r\n]|$)");

    QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    ipRe = QRegularExpression(ipRange + "\\." + ipRange + "\\." + ipRange + "\\." + ipRange);
}

LogMonitor::~LogMonitor()
{
    delete ts;
}

void LogMonitor::saveSettings()
{
    QVariantMap map;
    map.insert("log_file", file);
    map.insert("allowed_attempts", allowedAttempts);
    map.insert("attempt_reset_time_hr", attemptResetHr);
    map.insert("attempt_reset_time_min", attemptResetMin);
    map.insert("block_time_hr", blockHr);
    map.insert("block_time_min", blockMin);

    QJsonObject obj = QJsonObject::fromVariantMap(map);

    QJsonDocument settings(obj);

    QFile settingsFile(SETTINGS_FILE);

    if (!settingsFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "failed to open activity log file.";
        return;
    }

    settingsFile.write(settings.toJson(QJsonDocument::Indented));

    settingsFile.close();
}

void LogMonitor::readActivityLog()
{
    activityLog.setFileName(ATTEMPTS_FILE);

    QFileInfo check_file(ATTEMPTS_FILE);

    if (check_file.exists() && check_file.isFile()) // if the activity log exists, initialize json document
    {
        if (!activityLog.open(QIODevice::ReadOnly))
        {
            qDebug() << "failed to open activity log file.";
            return;
        }

        QTextStream stream(&activityLog);
        activityLogJson = QJsonDocument::fromJson(stream.readAll().toLocal8Bit());

        activityLog.close();
    }
}

void LogMonitor::saveActivityLog()
{
    if (!activityLog.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "failed to open activity log file.";
        return;
    }

    activityLog.write(activityLogJson.toJson(QJsonDocument::Indented));

    activityLog.close();
}

void LogMonitor::emptyActivityLog()
{
    if (!activityLog.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "failed to open activity log file.";
        return;
    }

    activityLogJson = QJsonDocument();  // sets to null

    activityLog.write(activityLogJson.toJson(QJsonDocument::Indented));

    activityLog.close();
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

        QJsonObject root = activityLogJson.object();
        QJsonArray array = root.value("login_attempts").toArray();

        // check if this IP has attempted login before
        for (int i = 0; i < array.size(); i++)
        {
            QJsonObject obj = array.at(i).toObject();

            if (obj["ip"] == ip)
            {
                int attempts = (obj["attempts"].toInt());

                if (type == ACCEPT)
                {
                    attempts = 0;
                }
                else if (type == FAILED)
                {
                    attempts += 1;
                }

                if (attempts > allowedAttempts)
                {
                    // ban ip here
                    status = "IP Blocked";
                }

                obj["time"] = time;
                obj["status"] = status;
                obj["program"] = "ssh";
                obj["attempts"] = attempts;

                array.removeAt(i);
                array.insert(i, obj);
                root.insert("login_attempts", array);
                activityLogJson.setObject(root);
                saveActivityLog();
                return;
            }
        }
        // attempt from new IP, add entry to activity log
        QVariantMap map;
        map.insert("ip", ip);
        map.insert("time", time);
        map.insert("status", status);
        map.insert("program", "ssh");
        map.insert("attempts", 1);

        QJsonObject obj = QJsonObject::fromVariantMap(map);
        array.insert(array.end(), obj);
        root.insert("login_attempts", array);
        activityLogJson.setObject(root);

        saveActivityLog();
    }
}

