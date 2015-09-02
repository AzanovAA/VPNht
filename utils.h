#pragma once

#include <QString>

#define SAFE_DELETE(x) if (x) { delete x; x = NULL; }

enum PROTOCOL_TYPE {  PPTP, L2TP, OpenVPN };

struct VpnProtocol{
    int id_;
    QString name_;
    QString label_;
};

struct VpnEncryption{
    int id_;
    QString name_;
    QString label_;
};

struct VpnAvailPort{
//    VpnAvailPort() : id_(-1) {}

    int id_;
    int vpnProtocol_;
    int vpnEncrypt_;
    QString sPorts;
};

struct ProxySetting {
    bool used_;
    QString server_;
    int port_;
    QString user_;
    QString password_;
};

extern VpnProtocol gVpnProtocol[];
extern VpnEncryption gVpnEncryption[];
extern VpnAvailPort gVpnAvailPorts[];

VpnProtocol getVpnProtocolByID(int id);
VpnProtocol getVpnProtocolByLabel(QString label);

VpnEncryption getVpnEncryptionByID(int id);
VpnEncryption getVpnEncryptionByLabel(QString label);

VpnAvailPort getAvailablePort(int protcolId, int encryptId);
QString generatePartUrlForPortEnc(int protocolId, int encryptId);

