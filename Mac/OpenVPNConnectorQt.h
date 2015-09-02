
#ifndef OPENVPNCONNECTORQT_H
#define OPENVPNCONNECTORQT_H

#include <QObject>

class OpenVPNConnectorQt : public QObject
{
    Q_OBJECT

public:
    explicit OpenVPNConnectorQt(QObject *parent = 0);
    virtual ~OpenVPNConnectorQt();

    bool installHelper(const QString &label);
    void connect(const QString &configPath, const QString &username, const QString &password);
    void disconnect();

    void executeRootCommand(const QString commandLine);

    enum ERROR {AUTH_ERROR, NO_OPENVPN_SOCKET, CANNOT_ALLOCATE_TUN_TAP};

    void emitConnected();
    void emitDisconnected();
    void emitStateChanged(const QString &state);
    void emitError(ERROR err);
    void emitLog(const QString &logStr);
    void emitStats(long bytesIn, long bytesOut);

signals:
    void connected();
    void disconnected();
    void stateChanged(const QString &state);
    void error(OpenVPNConnectorQt::ERROR err);
    void log(const QString &logStr);

    void statisticsUpdated(long bytesIn, long bytesOut);
};

Q_DECLARE_METATYPE(OpenVPNConnectorQt::ERROR);

#endif // OPENVPNCONNECTORQT_H
