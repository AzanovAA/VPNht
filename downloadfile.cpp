#include "downloadfile.h"
#include <QtNetwork>

DownloadFile::DownloadFile(QString url, QObject *parent)
    : QThread(parent), success_(false), url_(url)
{

}

DownloadFile::~DownloadFile()
{
    wait();
}

void DownloadFile::run()
{
	QNetworkAccessManager *manager = new QNetworkAccessManager();
	QEventLoop eventLoop;
	eventLoop.connect(manager, SIGNAL(finished(QNetworkReply *)), SLOT(quit()));

    QUrl url(url_);
    QNetworkRequest req(url);

    QSslConfiguration conf = req.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    conf.setProtocol(QSsl::TlsV1_0);
    req.setSslConfiguration(conf);

    QNetworkReply *reply = manager->get(req);
	eventLoop.exec();

	if (reply->error() != QNetworkReply::NoError)
	{
		success_ = false;
		return;
	}
	else
	{
		success_ = true;
	}
	
    data_ = reply->readAll();

    reply->deleteLater();
	manager->deleteLater();
}

