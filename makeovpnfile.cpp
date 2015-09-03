#include "makeovpnfile.h"
#include <QApplication>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

MakeOVPNFile::MakeOVPNFile()
{
}

/*bool MakeOVPNFile::generate(const QString &protocolPort, const QString &server, const QString &cipher, ProxySetting &ps, bool bSmartDNS)
{
    bMakeSuccess_ = false;

    QString OVPNorig = ":/MainWindow/ovpnconfs/config.ovpn";

    QStringList strs = protocolPort.split(" ");
    QStringList ports;
    QString protocol;
    if (strs.count() == 2) {
        protocol = strs[0];

        ports = strs[1].split(",");
    } else {
        Q_ASSERT(false);
        return false;
    }

    QFile configOrigFile(OVPNorig);
    if (!configOrigFile.open(QIODevice::ReadOnly)){
        Q_ASSERT(false);
        return false;
    }

    if (tempFile_.open())
    {
        tempFile_.resize(0);
        //qDebug() <<configOrigFile.readAll();
        tempFile_.write( configOrigFile.readAll() );
        // protocol
        if (protocol.contains("TCP", Qt::CaseInsensitive)) {
            tempFile_.write("proto tcp\r\n");
        } else {
            tempFile_.write("proto udp\r\n");
        }

        if (!bSmartDNS)
        {
            tempFile_.write("dhcp-option DNS 8.8.8.8\r\n");
            tempFile_.write("dhcp-option DNS 8.8.4.4\r\n");
        }

        // cipher
        QString str = "cipher " + cipher + "\r\n";
        tempFile_.write(str.toLocal8Bit());

        // server
        str.clear();
        QStringList::const_iterator constIterator;
        for (constIterator = ports.constBegin(); constIterator != ports.constEnd(); ++constIterator)
            str += "remote " + server + " " +(*constIterator).toLocal8Bit().constData() +"\r\n";
        tempFile_.write(str.toLocal8Bit());

        // proxy
        str.clear();
        if (ps.used_) {
            str = "http-proxy " + ps.server_ + " " + QString::number(ps.port_);

            if (!ps.user_.isEmpty())
                str += " stdin basic";
        }
        str += "\r\n";
        tempFile_.write(str.toLocal8Bit());

        tempFile_.close();
        configOrigFile.close();
        bMakeSuccess_ = true;
    }
    path_ = tempFile_.fileName();
    return bMakeSuccess_;
}*/

bool MakeOVPNFile::generate(const QString &ovpnData, ProxySetting &ps)
{
    bMakeSuccess_ = false;
    if (tempFile_.open())
    {
        tempFile_.resize(0);
        tempFile_.write( ovpnData.toUtf8() );


        // proxy
        QString str = "\r\n";
        if (ps.used_) {
            str += "http-proxy " + ps.server_ + " " + QString::number(ps.port_);

            if (!ps.user_.isEmpty())
                str += " stdin basic";
        }
        str += "\r\n";
        tempFile_.write(str.toLocal8Bit());

        tempFile_.close();
        bMakeSuccess_ = true;
    }
    path_ = tempFile_.fileName();
    return bMakeSuccess_;
}
