#include "getservers.h"
#include <QtNetwork>
#include <QSslConfiguration>
#include "downloadfile.h"
#include <QTextStream>
#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include "json/json.h"

GetServers::GetServers(QObject *parent)
    : QThread(parent), bExistData_(false), bErrorDuringDownload_(false)
{

}

void GetServers::setLoginData(QString user, QString password)
{
    user_ = user;
    password_ = password;
}

QVector<ServerInfo> GetServers::servers()
{
    if (bExistData_)
        return servers_;

    return QVector<ServerInfo>();
}


void GetServers::run()
{
    servers_.clear();
    bExistData_ = false;

    QNetworkAccessManager *manager = new QNetworkAccessManager();

	QEventLoop eventLoop;
	eventLoop.connect(manager, SIGNAL(finished(QNetworkReply *)), SLOT(quit()));

    QString strRequest = "https://api.vpn.ht/servers";

    QUrl url(strRequest);
    QNetworkRequest request(url);
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    conf.setProtocol(QSsl::TlsV1_0);
    request.setSslConfiguration(conf);

    request.setRawHeader("Authorization",
                         "Basic " + QByteArray(QString("%1:%2").arg(user_).arg(password_).toLatin1()).toBase64());

    QNetworkReply *reply = manager->get(request);
	eventLoop.exec();

	if (reply->error() != QNetworkReply::NoError)
	{
        int httpCodeErr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if ( httpCodeErr == 401)
        {
            emit authenticationError();
            return;
        }

        bErrorDuringDownload_ = true;
		errorStr_ = reply->errorString();
        emit updateFinished(false, errorStr_, false);
		return;
	}
	
    QVector<ServerInfo> downloadServers;
    QByteArray downloadConfigOVPN;

    QByteArray replyBytes = reply->readAll();
    //qDebug() <<replyBytes;

    /*****/
    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(replyBytes.constData(), root );
    if ( parsingSuccessful ) {
        Json::Value serversVal = root.get("servers", Json::Value::null);
        // array of vpn server info
        if ((serversVal != Json::Value::null) && serversVal.isArray()) {
            int size = serversVal.size();
            for ( int index =0; index < size; ++index ) {
                ServerInfo si;
                si.country_ = serversVal[index].get("c", Json::Value::null).asString().c_str();
                si.city_ = serversVal[index].get("l", Json::Value::null).asString().c_str();
                si.ip_ = serversVal[index].get("h", Json::Value::null).asString().c_str();
                si.name_ = si.ip_;
                si.name_.remove(si.ip_.length() - 7, 7);
                servers_ << si;
            }
        }
    }

    bExistData_ = true;

    emit updateFinished(true, QString(), true);

    reply->deleteLater();
    manager->deleteLater();
}
