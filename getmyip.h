#ifndef GETMYIP_H
#define GETMYIP_H

#include <QThread>

class GetMyIp : public QThread
{
    Q_OBJECT
public:
    GetMyIp(QObject *parent = 0);
    virtual ~GetMyIp();


signals:
    void myIpFinished(bool connected, QString myIp, QString countryCode);

protected:
    virtual void run();
};

#endif // GETMYIP_H
