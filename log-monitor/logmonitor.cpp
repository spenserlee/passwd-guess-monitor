#include "logmonitor.h"

LogMonitor::LogMonitor(QString path, QString attempts, QString resetHrs, QString resetMins, QString blockHr, QString blockMin)
{
    file = path;

    allowedAttempts = attempts.toInt();
    attemptResetHr  = resetHrs.toInt();
    attemptResetMin = resetMins.toInt();
    this->blockHr   = blockHr.toInt();
    this->blockMin  = blockMin.toInt();

    currentPath = QCoreApplication::applicationDirPath() + "/";

    saveSettings();
    readActivityLog();

    oldFileInfo = QFileInfo(file);
    logFile.setFileName(path);

    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "failed to open log file.";
        return;
    }

    startBlockMonitor();
    setIptablesUserChain();

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

    QFile settingsFile(currentPath + SETTINGS_FILE);

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
    activityLog.setFileName(currentPath + ATTEMPTS_FILE);

    QFileInfo check_file(currentPath + ATTEMPTS_FILE);

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

void LogMonitor::checkForUnbanState()
{
    if (activityLog.size() == 0)
    {
        activityLogJson = QJsonDocument();  // sets to null
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

void LogMonitor::startBlockMonitor()
{
    QThread *monitorThread;

    monitorThread   = new QThread(this);
    ipBlockMonitor  = new IpBlockMonitor(&activityLogJson, allowedAttempts, attemptResetHr, attemptResetMin, blockHr, blockMin);

    ipBlockMonitor->moveToThread(monitorThread);

    connect(ipBlockMonitor, SIGNAL(finished()), monitorThread, SLOT(quit()));
    connect(ipBlockMonitor, SIGNAL(finished()), ipBlockMonitor, SLOT(deleteLater()));
    connect(monitorThread, SIGNAL(finished()), monitorThread, SLOT(deleteLater()));

    connect(ipBlockMonitor, SIGNAL(saveActivityLog()), this, SLOT(saveActivityLog()));
    connect(monitorThread, SIGNAL(started()), ipBlockMonitor, SLOT(monitor()));

    monitorThread->start();
}

void LogMonitor::setIptablesUserChain()
{
    // create user chain
    QProcess::execute("iptables -N ip_block");

    // if it is not in the input chain already, add it
    if (QProcess::execute("iptables -C INPUT -j ip_block") == 1)
    {
        QProcess::execute("iptables -I INPUT -j ip_block");
    }
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

    // gui set to unblock all IPs

    int s = newFileInfo.size();

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
            checkForUnbanState();
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
        QString time = QString::number(QDateTime::currentSecsSinceEpoch());
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

                    // iptables block ip
                    system(QString("iptables -A ip_block -s " +  ip + " -j DROP").toUtf8().constData());
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

