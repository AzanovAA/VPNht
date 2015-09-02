#include "webrtcprotection.h"
#include <QDebug>

#if defined Q_OS_MAC
    #include "Mac/OpenVPNConnectorQt.h"
#elif defined Q_OS_WIN
    #include "Windows/OpenVPNConnectorQt.h"
#endif

extern OpenVPNConnectorQt *g_openVPNConnection;

WebRTCProtection::WebRTCProtection(QObject *parent) : QObject(parent), bEnable_(false)
{
}

WebRTCProtection::~WebRTCProtection()
{
    enable(false);
}

void WebRTCProtection::enable(bool bEnabled)
{
    if (bEnable_ == bEnabled)
        return;

    if (bEnabled)
    {
        QString cmd1 = "netsh advfirewall firewall add rule name=\"No-WebRTC\" dir=out action=block protocol=UDP localport=3478,19302";
        QString cmd2 = "netsh advfirewall firewall add rule name=\"No-WebRTC\" dir=out action=block protocol=UDP remoteport=3478,19302";
        QString cmd3 = "netsh advfirewall firewall add rule name=\"No-WebRTC\" dir=in action=block protocol=UDP localport=3478,19302";
        QString cmd4 = "netsh advfirewall firewall add rule name=\"No-WebRTC\" dir=in action=block protocol=UDP remoteport=3478,19302";

        quint32 exitCode1, exitCode2, exitCode3, exitCode4;
        bool b1 = g_openVPNConnection->executeRootCommand(cmd1, &exitCode1);
        bool b2 = g_openVPNConnection->executeRootCommand(cmd2, &exitCode2);
        bool b3 = g_openVPNConnection->executeRootCommand(cmd3, &exitCode3);
        bool b4 = g_openVPNConnection->executeRootCommand(cmd4, &exitCode4);
        qDebug() << "WebRTCProtection enabled : executed = " << (b1 && b2 && b3 && b4);

        bEnable_ = true;
    }
    else
    {
        QString cmd1 = "netsh advfirewall firewall delete rule name=\"No-WebRTC\" dir=out protocol=UDP localport=3478,19302";
        QString cmd2 = "netsh advfirewall firewall delete rule name=\"No-WebRTC\" dir=out protocol=UDP remoteport=3478,19302";
        QString cmd3 = "netsh advfirewall firewall delete rule name=\"No-WebRTC\" dir=in protocol=UDP localport=3478,19302";
        QString cmd4 = "netsh advfirewall firewall delete rule name=\"No-WebRTC\" dir=in protocol=UDP remoteport=3478,19302";

        quint32 exitCode1, exitCode2, exitCode3, exitCode4;
        bool b1 = g_openVPNConnection->executeRootCommand(cmd1, &exitCode1);
        bool b2 = g_openVPNConnection->executeRootCommand(cmd2, &exitCode2);
        bool b3 = g_openVPNConnection->executeRootCommand(cmd3, &exitCode3);
        bool b4 = g_openVPNConnection->executeRootCommand(cmd4, &exitCode4);
        qDebug() << "WebRTCProtection disabled : executed = " << (b1 && b2 && b3 && b4);

        bEnable_ = false;
    }
}

