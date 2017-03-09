#include "ipblockmonitor.h"

IpBlockMonitor::IpBlockMonitor(QObject *parent) : QObject(parent)
{

}

IpBlockMonitor::IpBlockMonitor(QJsonDocument *jsonDoc, int attempts, int attemptResetHr, int attemptResetMin, int blockHr, int blockMin)
{
    running = true;

    this->jsonDoc = jsonDoc;

    // seconds
    attemptResetTime    = (attemptResetHr * 3600) + (attemptResetMin * 60);
    blockTime           = (blockHr * 3600) + (blockMin * 60);
}

void IpBlockMonitor::monitor()
{
    while (running)
    {
        bool changesMade = false;

        if (jsonDoc->isEmpty() || jsonDoc->isNull())
        {
            sleep(1);
            continue;
        }

        // get the current time
        qint64 curTime = QDateTime::currentSecsSinceEpoch();


        QJsonObject root = jsonDoc->object();
        QJsonArray array = root.value("login_attempts").toArray();

        // loop through all of the items in the jsonDocument
        for (int i = 0; i < array.size(); i++)
        {
            QJsonObject obj = array.at(i).toObject();

        // if the status is Failed attempt
        //      if the current time - time for ip > attempt reset time
        //          set attempts to 0
            if (obj["status"] == "Failed Attempt")
            {
                qint64 time = obj["time"].toString().toLongLong();

                qint64 diff = curTime - time;

                if (diff > attemptResetTime)
                {
                    obj["attempts"] = 0;
                    obj["status"] = "Attempts Reset";
                    changesMade = true;
                }
            }
        // else if status is IP Blocked
        //      if the current time - time for ip > block time
        //          set attempts to 0
        //          unblock the IP
            else if (obj["status"] == "IP Blocked")
            {
                qint64 time = obj["time"].toString().toLongLong();

                qint64 diff = curTime - time;

                if (diff > blockTime)
                {
                    obj["attempts"] = 0;
                    obj["status"] = "IP Block Expired";

                    changesMade = true;

                    // iptables unblock
                    system(QString("iptables -D ip_block -s " +  obj["ip"].toString() + " -j DROP").toUtf8().constData());
                }
            }
            if (changesMade)
            {
                array.removeAt(i);
                array.insert(i, obj);
            }
        }

        if (changesMade)
        {
            root.insert("login_attempts", array);
            jsonDoc->setObject(root);
            emit saveActivityLog();
        }

        sleep(1);
    }
    emit finished();
}

void IpBlockMonitor::stopMonitoring()
{
    running = false;
}
