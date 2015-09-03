#ifndef MAKEOVPNFILE_H
#define MAKEOVPNFILE_H
#include "utils.h"
#include <QTemporaryFile>


class MakeOVPNFile
{
public:
    MakeOVPNFile();

    //bool generate(const QString &protocolPort, const QString &server, const QString &cipher, ProxySetting &ps, bool bSmartDNS);
    bool generate(const QString &ovpnData, ProxySetting &ps);

    QString path() { return path_; }
    bool makeSuccess() { return bMakeSuccess_; }
private:
    QTemporaryFile tempFile_;
    QString path_;
    bool bMakeSuccess_;
};

#endif // MAKEOVPNFILE_H
