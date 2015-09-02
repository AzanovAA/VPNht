#include "../smartdns.h"

#include <QDebug>
#include <QProcess>
#include <QEventLoop>

#include "OpenVPNConnectorQt.h"

extern OpenVPNConnectorQt *g_openVPNConnection;
const QString smartDnsVPNht = "178.62.106.191,104.131.49.85";
const QString smartDnsGoogle = "7.7.7.7,8.8.8.8";

QList<QString> getServiceList()
{
    QList<QString> lAdapters;
    QProcess process;
    QStringList params;
    params << "-listnetworkserviceorder";

    process.start("networksetup", params);

    QEventLoop eventLoop;
    eventLoop.connect(&process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(quit()));
    eventLoop.exec();

    // read console output
    int i = 0;
    QList<QByteArray> data = process.readAll().split('\n');
    while (i < data.count()) {
        if ( data[i].startsWith("(") ) {
            QList<QByteArray> adaptInfo = data[i+1].split(',');
            QString adapter = adaptInfo[0].split(':')[1].trimmed(); //
            qDebug() <<"find adpter " <<adapter;
            lAdapters.append( adapter );
            i += 2;
        }
        i++;
    }

    return lAdapters;
}

QList<QString> getDnsOnAdapter(QString adapter)
{
    QList<QString> adapters;
    QProcess process;
    QStringList params;
    params << "-getdnsservers" <<adapter;


    process.start("networksetup", params);

    QEventLoop eventLoop;
    eventLoop.connect(&process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(quit()));
    eventLoop.exec();

    int i = 0;
    QList<QByteArray> data = process.readAll().split('\n');
    while (i < data.count()) {
        if (!data[i].startsWith("There") && !data[i].isEmpty())
            adapters.append( data[i] );
        i++;
    }

    return adapters;
}

void setDnsOnAdapter(QString adapter, QList<QString> dns)
{
    QString commandLine = "networksetup -setdnsservers '" + adapter + "'";
    foreach (QString s, dns)
        commandLine += " " + s;

    g_openVPNConnection->executeRootCommand(commandLine);
    qDebug() <<"Set dns on Adapter: " <<adapter <<"DNS: " <<dns;
}


SmartDns::SmartDns() : smartDnsSwitchActive_(false), dnsLeakSwitchActive_(false)
{

}

SmartDns::~SmartDns()
{
    dnsLeakEnable(false);
}

void SmartDns::enableDnsFeatures(bool smartDnsOn, bool dnsLeakOn, bool usedVpn, bool afterConnecting)
{
    smartDnsEnabled(smartDnsOn, usedVpn, afterConnecting);

    dnsLeakEnable(dnsLeakOn && usedVpn);    // dns leak active only when vpn connecting

    smartDnsSwitchActive_ = DnsOn;
    dnsLeakSwitchActive_ = (dnsLeakOn && usedVpn);
}

void SmartDns::smartDnsEnabled(bool enabled, bool usedVpn, bool afterConnecting)
{
    if ((smartDnsSwitchActive_ == enabled) && (!usedVpn)) {
        reStoreOldDns(oldDns_);
        return;
    }

    QMap<QString, QString> dns;
    if (enabled) {
        if (usedVpn && afterConnecting)
            assignNewDns(smartDnsVPNht, oldDns_, All);
        else if (usedVpn)
            assignNewDns(smartDnsVPNht, dns, All);
        else
            assignNewDns(smartDnsVPNht, oldDns_, All);

    }else {
        if (usedVpn)
            assignNewDns(smartDnsGoogle, dns, All);
        else
            reStoreOldDns(oldDns_);

    }
}

bool SmartDns::assignNewDns(const QString &newDns, QMap<QString, QString> &dns, SelectAdapter mode)
{
    QList<QString> lAdapter = getServiceList();
    foreach (QString adapter, lAdapter) {
        QList<QString> lDns = getDnsOnAdapter(adapter);
        if (lDns.isEmpty()) {
            dns.insert(adapter, "empty");
        }else {
            foreach (QString sDns, lDns) {
                dns.insertMulti(adapter, sDns);
            }
        }


        setDnsOnAdapter(adapter, newDns.split(','));
    }
}

void SmartDns::reStoreOldDns(QMap<QString, QString> &dns)
{
    foreach (QString adapter, dns.uniqueKeys()) {
        setDnsOnAdapter(adapter, dns.values(adapter));
    }
}


void SmartDns::dnsLeakEnable(bool enable)
{

}
