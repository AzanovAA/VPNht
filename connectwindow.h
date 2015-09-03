#ifndef CONNECTWINDOW_H
#define CONNECTWINDOW_H

#include <QWidget>
#include <QItemDelegate>
#include "ui_connectwindow.h"

#if defined Q_OS_WIN
#include "Windows/OpenVPNConnectorQt.h"
#elif defined Q_OS_MAC
#include "Mac/OpenVPNConnectorQt.h"
#endif

#include "showlog.h"
#include "getservers.h"
#include "utils.h"
#include "getmyip.h"
#include "makeovpnfile.h"
#include "downloadfile.h"
#include "dnsleaks.h"

class MainWindow;
class KillSwitch;
class ShowLog;

class ConnectWindow : public QWidget
{
	Q_OBJECT
public:
    ConnectWindow(QMenu *settingsMenu, QWidget *parent= 0);
	~ConnectWindow();

	void setServers(const QVector<ServerInfo> &servers);
    void setAcountData(QString user, QString password);
	void loadSettings();
	void saveSettings();
	void autoConnect();
    bool isConnected();
    void disconnectConnection();
    void showLog();

protected:
	void timerEvent(QTimerEvent *event);
    bool eventFilter(QObject* object, QEvent* event);

signals:
    void serverList();
    void connected();

public slots:
    void onServerChanged(const QString & server);

private slots:
	void onClickConnect();
    void onSmartDnsHelp();
	void onConnected();
    void onDisconnected();
    void onConnectionError(OPENVPN_ERROR);
    void onClickSmartDns();
    void onCancelConnect();
    void onMyIpUpdated(bool connected, QString myIp, QString countryCode);
    void onRestoreGateway();
    void onStatisticsChanged(long bytesIn, long bytesOut);

    void onDNSEnabledStateChanged(int state);

private:
	Ui::ConnectWindow ui;
    enum {STATE_CONNECTED, STATE_DISCONNECTED, STATE_CONNECTING, STATE_DISCONNECTING};
	int state_;
    enum {MODE_OPEN_VPN, MODE_SMART_DNS};
    int mode_;
    DnsLeaks dnsLeaks_;
	QMap<QString, ServerInfo>  servers_;
    int timerVpnFeatures_;
    QString password_;
    QString userName_;    

    MainWindow *mainWindow_;
    QPushButton *pbCancelCon;
    QPushButton *pbRestoreGateway;
    KillSwitch *killSwitch_;
    ShowLog *logDialog_;

    GetMyIp *getMyVpnIp_;

    MakeOVPNFile ovpnFile_;

    long prevBytesIn;
    long prevBytesOut;
    bool bFirstBytes;

//    bool dnsLeakSwitchActive_;
//    bool smartDnsSwitchActive_;

    //void setEnabledSmartDNS(bool enabled);
    void setSwitchOnSmartDNS(bool used);
    void setBarInConnectstatus(bool enabled);
    void showStatistics(bool enabled);
    QString formatTraficStr(double trafic, QString prefix = 0);
    void createButtonRestoreGw();
    void disableVpnFeatures();
    void updateIP();

};
#endif // CONNECTWINDOW_H
