#include "getmyip.h"
#include <QThread>
#include <QtNetwork>
#include "json/json.h"
#include <QDebug>

GetMyIp::GetMyIp(QObject *parent) :
    QThread(parent)
{

}

GetMyIp::~GetMyIp()
{
}

void GetMyIp::run()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager();

    QEventLoop eventLoop;
    eventLoop.connect(manager, SIGNAL(finished(QNetworkReply *)), SLOT(quit()));

    QString strRequest = "https://myip.ht/status";

    QUrl url(strRequest);
    QNetworkRequest request(url);
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    conf.setProtocol(QSsl::TlsV1_0);
    request.setSslConfiguration(conf);

    QNetworkReply *reply = manager->get(request);
    eventLoop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        emit myIpFinished(false, "0.0.0.0", "");
        return;
    }

    QByteArray replyBytes = reply->readAll();

    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(replyBytes.constData(), root );
    if ( parsingSuccessful ) {
        Json::Value isConnected = root.get("connected", Json::Value::null);
        //if ((isConnected != Json::Value::null) && isConnected.asBool()) {
            Json::Value myIp = root.get("ip", Json::Value::null);
            emit myIpFinished(true, myIp.asString().c_str(), root.get("country", Json::Value::null).asString().c_str());
        //}else {
        //    emit myIpFinished(false, "0.0.0.0", "");
        //}
    }
}
