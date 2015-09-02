#include "utils.h"

VpnProtocol gVpnProtocol[] = {
    {0, "udp", "Udp"},
    {1, "tcp", "Tcp"},
    {2, "udp", "Udp 53"},
    {3, "udp", "Udp 3389"},
    {4, "tcp", "Tcp 80"},
    {5, "tcp", "Tcp 443"},
    {-1}
};

VpnEncryption gVpnEncryption[] = {
    {0, "BF-CBC", "64bit  BlowFish"},
    {1, "AES-128-CBC", "128 bit AES"},
    {2, "AES-256-CBC", "256 bit AES"},
    {-1}
};

VpnAvailPort gVpnAvailPorts[] = {
    {0, 0, 0, "1202,1203,1204,1205"}, // UDP 64bit
    {1, 0, 1, "1194,1195,1196,1197,1198,1199,1200,1201"}, // UDP 128bit
    {3, 2, 1, "53"}, // UDP 53 128bit
    {3, 3, 1, "3389"}, // UDP 53 128bit
    {5, 4, 1, "80"}, // TCP 80 is 128 BIT
    {6, 5, 1, "443"}, // TCP 443 is 128 BIT
    {2, 0, 2, "1300,1301,1302,1303,1304"}, // UDP 256bit
    {4, 3, 2, "3389"}, // UDP 3389 is 256 BIT
    {5, 2, 2, "53"}, // TCP 80 is 256 BIT
    {-1}
};

VpnProtocol getVpnProtocolByID(int id)
{
    int i;
    for(i=0; gVpnProtocol[i].id_!=-1; i++) {
        if (gVpnProtocol[i].id_ == id)
            return gVpnProtocol[i];
    }

    return gVpnProtocol[i];
}

VpnProtocol getVpnProtocolByLabel(QString label)
{
    int i;
    for( i=0; gVpnProtocol[i].id_!=-1; i++) {
        if (gVpnProtocol[i].label_ == label)
            return gVpnProtocol[i];
    }

    return gVpnProtocol[i];
}

VpnEncryption getVpnEncryptionByID(int id)
{
    int i;
    for( i=0; gVpnEncryption[i].id_!=-1; i++) {
        if (gVpnEncryption[i].id_ == id)
            return gVpnEncryption[i];
    }

    return gVpnEncryption[i];
}

VpnEncryption getVpnEncryptionByLabel(QString label)
{
    int i;
    for( i=0; gVpnEncryption[i].id_!=-1; i++) {
        if (gVpnEncryption[i].label_ == label)
            return gVpnEncryption[i];
    }

    return gVpnEncryption[i];
}

VpnAvailPort getAvailablePort(int protcolId, int encryptId)
{
    int i;
    for( i=0; gVpnAvailPorts[i].id_!=-1; i++) {
        if ( (gVpnAvailPorts[i].vpnProtocol_ == protcolId)
                && (gVpnAvailPorts[i].vpnEncrypt_ == encryptId) )
            return gVpnAvailPorts[i];
    }

    return gVpnAvailPorts[i];
}


QString generatePartUrlForPortEnc(int protocolId, int encryptId)
{
    if (protocolId == 0 && encryptId == 0)
        return "/64";
    else if (protocolId == 0 && encryptId == 1)
        return "/128";
    else if (protocolId == 0 && encryptId == 2)
        return "/256";
    else if (protocolId == 2 && encryptId == 1)
        return "/53/128";
    else if (protocolId == 4 && encryptId == 1)
        return "/80/128";
    else if (protocolId == 5 && encryptId == 1)
        return "/443/128";
    else if (protocolId == 3 && encryptId == 1)
        return "/3389/128";
    else if (protocolId == 3 && encryptId == 2)
        return "/3389/256";
    else if (protocolId == 2 && encryptId == 2)
        return "/53/256";

    Q_ASSERT(false);
    return "";
}
