#include "pingwrapper.h"

#include <QDebug>
#include <QTextCodec>

PingWrapper::PingWrapper(QString host, QObject *parent)
    : QThread(parent), host_(host)
{
}


void PingWrapper::run()
{    
    QProcess process(this);
    QStringList params;

#if defined Q_OS_WIN
    params << "-n" <<"1" <<host_;
#elif defined Q_OS_MAC
    params << "-c" <<"1" <<host_;
#endif

    process.start("ping", params);
    process.waitForFinished();

#if defined Q_OS_WIN
    QTextCodec *codec = QTextCodec::codecForName("CP866");  //Windows-1251 CP1251 KOI8-R
    QString data = codec->toUnicode(process.readLine());
    while (!data.isEmpty()) {
        QStringList list = data.split(" ");
        foreach(QString s, list) {
            if ( (s.contains("ms", Qt::CaseInsensitive))
                  || (s.contains(QString::fromUtf8("мс"), Qt::CaseInsensitive)) ) {
                QStringList lTime = s.split("=");
                if (lTime.size() > 1) {
                    QString strTime = lTime[1];
                    strTime.remove(QString::fromUtf8("мс"));
                    strTime.remove("ms");
                    emit pingFinished(true, strTime.toDouble(), host_);
                    return;
                }
            }
        }

        data = codec->toUnicode(process.readLine());
    }
#elif defined Q_OS_MAC
    QString data = process.readLine();
    while (!data.isEmpty()) {
        QStringList list = data.split(" ");
        foreach(QString s, list) {
            if (s.contains("time", Qt::CaseInsensitive)) {
                QStringList lTime = s.split("=");
                if (lTime.size() > 1) {
                    QString strTime = lTime[1];
                    strTime.remove(QString::fromUtf8("мс"));
                    strTime.remove("ms");
                    emit pingFinished(true, strTime.toDouble(), host_);
                    return;
                }
            }
        }

        data = process.readLine();
    }
#endif        

    emit pingFinished(false, 0.0, host_);
}

