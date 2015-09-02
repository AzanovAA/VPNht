#pragma once

#include <QThread>
#include <QXmlStreamReader>
#include <QStringList>
#include <QMutex>

struct ServerInfo
{
    QString country_;
    QString city_;
    QString ip_;
    QString name_;
};

class QNetworkReply;
class QAuthenticator;

class GetServers : public QThread
{
	Q_OBJECT

public:
	GetServers(QObject *parent = NULL);

    QVector<ServerInfo> servers();
    void setLoginData(QString user, QString password);

signals:
    void updateFinished(bool bSuccess, QString errorStr, bool bConfigChanged);
    void authenticationError();

protected:
    virtual void run();

private:
    bool	bExistData_;
    bool    bErrorDuringDownload_;
	QString errorStr_;
	QVector<ServerInfo> servers_;
    QByteArray configOVPN_;
    QString user_;
    QString password_;
};

