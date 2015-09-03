#ifndef DNSLEAKS_H
#define DNSLEAKS_H
#include <QMap>

class DnsLeaks
{
public:
    DnsLeaks();
    virtual ~DnsLeaks();
    void enable(bool bEnabled);
    bool isEnable() { return bEnable_; }

private:
    bool bEnable_;

#if defined Q_OS_WIN
    QMap<QString, QString> oldDnsLeak_;

    void assignNewDns(const QString &newDns, QMap<QString, QString> &dns);
    void restoreOldDns(QMap<QString, QString> &dns);
    bool regSetDNS(const QString &lpszAdapterName, const QString &pDNS);
    bool notifyIPChange(const char *lpszAdapterName);
    bool flushDNS();
#endif
};

#endif // DNSLEAKS_H
