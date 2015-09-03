#include "connectwindow.h"
#include "log.h"
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QMouseEvent>
#include <QDesktopServices>
#include "mainwindow.h"
#include "downloadfile.h"
#include "killswitch.h"
#include "makeovpnfile.h"

extern OpenVPNConnectorQt *g_openVPNConnection;

ConnectWindow::ConnectWindow(QMenu *settingsMenu, QWidget *parent)
    : QWidget(parent),
      state_(STATE_DISCONNECTED),
      pbCancelCon(0),
      pbRestoreGateway(0),
      killSwitch_(NULL),
      getMyVpnIp_(NULL),
      smartDNS_OVPNFile_("https://vpn.ht/openvpn-config/hub?smartdns-only"),
      mode_(MODE_OPEN_VPN),
      logDialog_(NULL)
{
    ui.setupUi(this);

    smartDNS_OVPNFile_.start(QThread::LowPriority);

    QString strUpdatedStyleSheet;
    QString strStyleSheet = ui.tabWidget->styleSheet();
    QString matchStr = "QTabWidget:tab-bar{}";
    int ind = strStyleSheet.indexOf(matchStr);
    Q_ASSERT(ind != -1);
    strUpdatedStyleSheet = strStyleSheet.left(ind);
    strUpdatedStyleSheet += "QTabWidget:tab-bar{left: 100px; top: 10px;}";
    strUpdatedStyleSheet += strStyleSheet.right(strStyleSheet.length() - (ind + matchStr.length()));
    ui.tabWidget->setStyleSheet(strUpdatedStyleSheet);

    ui.btnSettings->setMenu( settingsMenu );

    showStatistics( false );

    mainWindow_ = (MainWindow *)parent;

    killSwitch_ = new KillSwitch(this);

    QSettings settings;
    ui.cbEnableSmartDNS->setChecked(settings.value("openVpnDNSEnabled", true).toBool());

    /*if (!connection_.initialize())
	{
		QMessageBox::information(this, QApplication::applicationName(), tr("OpenVPN initialize error"));
		QTimer::singleShot(1, parent, SLOT(close()));
		return;
	}

    if (!connection_.tapInstalled())
	{
		QMessageBox::question(this, QApplication::applicationName(), tr("TAP drivers not installed."));
		QTimer::singleShot(1, this, SLOT(close()));
		return;
    }   */

    ui.cbServer->installEventFilter(this);

    logDialog_ = new ShowLog(this);
    connect(g_openVPNConnection, SIGNAL(log(QString)), logDialog_, SLOT(onUpdateLog(QString)));

    updateIP();

    qRegisterMetaType<OPENVPN_ERROR>("OPENVPN_ERROR");

	connect(ui.btnConnect, SIGNAL(clicked()), SLOT(onClickConnect()));
    connect(ui.cbEnableSmartDNS, SIGNAL(stateChanged(int)), SLOT(onDNSEnabledStateChanged(int)));
    connect(ui.pbSmartDnsHelp, SIGNAL(clicked()), SLOT(onSmartDnsHelp()));
    connect(ui.pbSmartDns, SIGNAL(clicked()), SLOT(onClickSmartDns()));
    connect(g_openVPNConnection, SIGNAL(connected()), SLOT(onConnected()));
    connect(g_openVPNConnection, SIGNAL(disconnected()), SLOT(onDisconnected()));
    connect(g_openVPNConnection, SIGNAL(error(OPENVPN_ERROR)), SLOT(onConnectionError(OPENVPN_ERROR)));
    connect(g_openVPNConnection, SIGNAL(disconnected()), SLOT(onDisconnected()));
    connect(g_openVPNConnection, SIGNAL(statisticsUpdated(long, long)), SLOT(onStatisticsChanged(long, long)));
}

ConnectWindow::~ConnectWindow()
{
    if (getMyVpnIp_)
    {
        getMyVpnIp_->terminate();
        getMyVpnIp_->wait();
        getMyVpnIp_->deleteLater();
    }
    g_openVPNConnection->disconnect();

    disableVpnFeatures();

    SAFE_DELETE( pbCancelCon );
    SAFE_DELETE( pbRestoreGateway );
    SAFE_DELETE( killSwitch_ );
}

void ConnectWindow::onClickConnect()
{
	QSettings settings;

    mode_ = MODE_OPEN_VPN;

	if (state_ == STATE_DISCONNECTED)
    {
        mainWindow_->showNormal();
        mainWindow_->activateWindow();

        ServerInfo si = servers_[ui.cbServer->text()];
        if (si.ip_.isEmpty() || si.country_.isEmpty())
        {
            QMessageBox::information((QWidget *)parent(),
                                     QApplication::applicationName(), "Server not selected");
            return;
        }

        PROTOCOL_TYPE protocol;
        protocol = OpenVPN;
        VpnProtocol prot = getVpnProtocolByLabel( settings.value("vpnProtocol").toString() );
        VpnEncryption enc = getVpnEncryptionByLabel( settings.value("vpnEncryption").toString() );

        //VpnAvailPort availPorts = getAvailablePort(prot.id_, enc.id_);

        ProxySetting ps;
        ps.used_ = settings.value("proxyIsOn").toBool();
        if (ps.used_) {

            if (!prot.name_.contains("tcp", Qt::CaseInsensitive))
            {
                QMessageBox::information((QWidget *)parent(),
                                         QApplication::applicationName(), "UDP protocol not support proxy. Please change protocol to TCP or disable Proxy");
                return;
            }

            ps.server_ = settings.value("proxyUrl").toString();
            ps.port_ = settings.value("proxyPort").toInt();
            ps.user_ = settings.value("proxyUser").toString();
            ps.password_ = settings.value("proxyPassword").toString();
        }

        ui.btnConnect->setStyleSheet("QPushButton { border-image :url(:/MainWindow/Images/btnOff.png); }");
        ui.lbStatusCon->setText(QString("<b>Status:  </b><font color=\"green\">Connecting</font>"));

        state_ = STATE_CONNECTING;

        if (killSwitch_->isActive())
        {
            killSwitch_->restoreInternet();
        }
        if (settings.value("vpnKill").toString() == "true")
        {
            killSwitch_->saveDefaultGateway();
        }
        SAFE_DELETE (pbRestoreGateway)

        logDialog_->clear();

        mainWindow_->setTrayStatusConnecting();
        ui.cbServer->setEnabled( false );
        ui.cbEnableSmartDNS->setEnabled( false );

        ui.tab_2->setEnabled(false);

        setBarInConnectstatus(true);
        QApplication::processEvents();

        QString portEncPars = generatePartUrlForPortEnc(prot.id_, enc.id_);
        QString urlGen = "https://vpn.ht/openvpn-config/" + si.name_ + portEncPars;
        if (!ui.cbEnableSmartDNS->isChecked())
        {
            urlGen += "?disable-smartdns";
        }
        DownloadFile ovpnConfig(urlGen);
        ovpnConfig.start();
        ovpnConfig.wait();

        if (!ovpnConfig.success())
        {
            state_ = STATE_DISCONNECTED;
            setBarInConnectstatus(false);
            ui.lbStatusCon->setText(QString("<b>Status: </b><font color=\"red\">Disconnected</font>"));
            ui.cbServer->setEnabled( true );
            ui.cbEnableSmartDNS->setEnabled( true );
            ui.tab_2->setEnabled(true);
            QMessageBox::information(this, QApplication::applicationName(), "Can't download config file. Please contact support.");
            return;
        }

        // delete log file
        ALog::Clear();
		
        ovpnFile_.generate(ovpnConfig.data());
        //ovpnFile_.generate(QString("%1 %2").arg(prot.name_).arg(availPorts.sPorts),
         //                     si.ip_, enc.name_, ps, ui.cbEnableSmartDNS->isChecked());

#if defined Q_OS_WIN
        QString ovpnPath = ovpnFile_.path();
#elif defined Q_OS_MAC
        QString ovpnPath = "\'" + ovpnFile_.path() + "\'";
#endif
        g_openVPNConnection->connect(ovpnPath, userName_, password_, ps.user_, ps.password_);

	}
    else
	{
        state_ = STATE_DISCONNECTING;
        ui.btnConnect->setEnabled(false);
        g_openVPNConnection->disconnect();
    }
}

void ConnectWindow::onSmartDnsHelp()
{
    QString link = "https://vpn.ht/channels";
    QDesktopServices::openUrl(QUrl(link));
}

void ConnectWindow::onConnected()
{
    prevBytesIn = 0;
    prevBytesOut = 0;
    bFirstBytes = true;

    state_ = STATE_CONNECTED;
    mainWindow_->setTrayStatusConnected();
    setBarInConnectstatus(false);

    if (mode_ == MODE_OPEN_VPN)
    {
        ui.btnConnect->setStyleSheet("QPushButton {border-image :url(:/MainWindow/Images/btnOn.png);}");
        showStatistics(true);
        ui.cbServer->setEnabled( false );

        ui.lbStatusCon->setText(QString("<b>Status:  </b><font color=\"green\">Connected</font>"));
        updateIP();

        QSettings settings;
        if (settings.value("dnsLeakProtect", false).toBool())
        {
            dnsLeaks_.enable(true);
        }
    }
    else
    {
        QSettings settings;
        if (settings.value("dnsLeakProtect", false).toBool())
        {
            dnsLeaks_.enable(true);
        }

        setSwitchOnSmartDNS( true );
    }

/*#if defined Q_OS_WIN
    timerVpnFeatures_ = startTimer(1500);
#elif defined Q_OS_MAC
    timerVpnFeatures_ = startTimer(1);
#endif*/

    emit connected();
}

void ConnectWindow::onDisconnected()
{
    QSettings settings;
    if (mode_ == MODE_OPEN_VPN)
    {
        ui.btnConnect->setStyleSheet("QPushButton {"
            "border-image :url(:/MainWindow/Images/btnOff.png);}");
        ui.btnConnect->setEnabled(true);

        if ((settings.value("vpnKill").toString() == "true") && !killSwitch_->isActive()
            && (state_ == STATE_CONNECTED))
        {
            killSwitch_->killInternet();
            createButtonRestoreGw();
        }

        if (settings.value("dnsLeakProtect", false).toBool())
        {
            dnsLeaks_.enable(false);
        }

        state_ = STATE_DISCONNECTED;

        setBarInConnectstatus(false);
        showStatistics(false);

        ui.cbServer->setEnabled( true );
        ui.cbEnableSmartDNS->setEnabled( true );
        ui.lbStatusCon->setText(QString("<b>Status: </b><font color=\"red\">Disconnected</font>"));

        updateIP();

        ui.tab->setEnabled(true);
        ui.tab_2->setEnabled(true);
        mainWindow_->setTrayStatusDisconnected();
    }
    else
    {
        if (settings.value("dnsLeakProtect", false).toBool())
        {
            dnsLeaks_.enable(false);
        }

        state_ = STATE_DISCONNECTED;
        setBarInConnectstatus(false);
        setSwitchOnSmartDNS(false);
        ui.pbSmartDns->setEnabled(true);

        ui.tab->setEnabled(true);
        ui.tab_2->setEnabled(true);
        mainWindow_->setTrayStatusDisconnected();
    }
}


void ConnectWindow::onConnectionError(OPENVPN_ERROR err)
{
    if (err == AUTH_ERROR)
    {
        QMessageBox::information((QWidget *)parent(),
                                 "VPNht", "Authorization failed");
    }
    else if (err == PROXY_AUTH_ERROR)
    {
        QMessageBox::information((QWidget *)parent(),
                                 "VPNht", "Proxy authorization failed");
    }
    else if (err == CANNOT_ALLOCATE_TUN_TAP)
    {
        QMessageBox::information((QWidget *)parent(), "VPNht",
                    "There are no TAP-Windows adapters on this system. Please install.");
    }
    else
    {
        QMessageBox::information((QWidget *)parent(),
                                 "VPNht", "Error during connection number: " + QString::number(err));
    }

    ui.btnConnect->setStyleSheet("QPushButton {"
        "border-image :url(:/MainWindow/Images/btnOff.png);}");

    state_ = STATE_DISCONNECTED;    

    setBarInConnectstatus(false);
    showStatistics(false);
    ui.cbServer->setEnabled( true );
    ui.cbEnableSmartDNS->setEnabled( true );
    ui.lbStatusCon->setText(QString("<b>Status: </b><font color=\"red\">Disconnected</font>"));

    ui.tab->setEnabled(true);
    ui.tab_2->setEnabled(true);
    ui.pbSmartDns->setEnabled(true);
    ui.btnConnect->setEnabled(true);
    mainWindow_->setTrayStatusDisconnected(false);
}


void ConnectWindow::onMyIpUpdated(bool connected, QString myIp, QString countryCode)
{
    ui.lbIpAdress->setText(QString("<b>IP: </b>%1").arg(myIp));
    ui.lbFlag->setPixmap( QPixmap( QString(":/MainWindow/Images/flags/%1.png").arg(countryCode.toUpper()) ) );
}

void ConnectWindow::onCancelConnect()
{
    if (state_ == STATE_CONNECTING)
    {
        state_ = STATE_DISCONNECTING;
        if (mode_ == MODE_OPEN_VPN)
            ui.btnConnect->setEnabled(false);
        else
            ui.pbSmartDns->setEnabled(false);
        g_openVPNConnection->disconnect();
    }
}


/*void ConnectWindow::setEnabledSmartDNS(bool enabled)
{
    ui.pbSmartDns->setEnabled( enabled );
}*/

void ConnectWindow::setSwitchOnSmartDNS(bool used)
{
    if (!used) {
        ui.lbSmartDnsStatus->setText(QString("<b>Status:  </b><font color=\"red\">Disable</font>"));
        ui.pbSmartDns->setStyleSheet("QPushButton{"
            "border-image: url(:/MainWindow/Images/btnOff.png);}"
        );
    } else {        
        ui.lbSmartDnsStatus->setText(QString("<b>Status:  </b><font color=\"green\">Enable</font>"));
        ui.pbSmartDns->setStyleSheet("QPushButton{"
            "border-image: url(:/MainWindow/Images/btnOn.png);}"
        );
    }
}

void ConnectWindow::setBarInConnectstatus(bool enabled)
{
    if (enabled) {
        ui.label->setVisible( false );
        ui.btnSettings->setVisible( false );
        ui.footer->setStyleSheet("QWidget#footer{"
            "background-image: url(:/MainWindow/Images/BackGroundBarOnCon.png);}"
        );
        pbCancelCon = new QPushButton(ui.footer);
        connect(pbCancelCon, SIGNAL(clicked()), SLOT(onCancelConnect()));
        pbCancelCon->setGeometry(250, 15, 110, 30);//(250, 15, 110, 30);
        pbCancelCon->setStyleSheet("QPushButton{"
                                   "border-radius: 6px;"
                                   "font: 12pt \"Marid Pro\";"
                                   "color: rgb(255, 255, 255);"
                                   "background-color: red;}"
        );
        pbCancelCon->setText("Cancel");
        pbCancelCon->setVisible( true );
    } else {
        SAFE_DELETE (pbCancelCon)

        ui.label->setVisible( true );
        ui.btnSettings->setVisible( true );
        ui.footer->setStyleSheet("QWidget#footer{"
            "background-image: url(:/MainWindow/Images/BackGroundBar.png);}");
    }
}

void ConnectWindow::setServers(const QVector<ServerInfo> &servers)
{
	ui.cbServer->clear();
	servers_.clear();
    foreach(const ServerInfo &si, servers)
	{
        servers_[si.country_] = si;
	}

    loadSettings();
}

void ConnectWindow::setAcountData(QString user, QString password)
{
    userName_ = user;
    password_ = password;    
}

void ConnectWindow::showStatistics(bool enabled)
{
    ui.cbServer->setVisible( !enabled );

    ui.lbIn->setVisible( enabled );
    ui.lbDownloadSpeed->setVisible( enabled );
    ui.lbDownloadBytes->setVisible( enabled );

    ui.lbOut->setVisible( enabled );
    ui.lbUploadSpeed->setVisible( enabled );
    ui.lbUploadBytes->setVisible( enabled );

    if (enabled) {
        ui.lbDownloadSpeed->setText("0.00 B/s");
        ui.lbUploadSpeed->setText("0.00 B/s");
        ui.lbDownloadBytes->setText("0.00 B");
        ui.lbUploadBytes->setText("0.00 B");
    }
}

QString ConnectWindow::formatTraficStr(double trafic, QString prefix)
{
    double data = 0.0;
    QString measure = "B";

    if (trafic < 1024) { // bytes
        data = trafic;
        measure = " B";
    } else if (trafic < (1024*1024)) { //KBytes
        data = trafic / 1042;
        measure = " KB";
    } else if (trafic < (1024*1024*1024)) {  //MBytes
        data = trafic / (1042*1024);
        measure = " MB";
    }

    return QString::number(data, 'f', 2) + measure + prefix;
}

void ConnectWindow::onServerChanged(const QString &server)
{
    ui.cbServer->setText( server );
}


void ConnectWindow::timerEvent(QTimerEvent *event)
{
    /*if (event->timerId() == timerVpnFeatures_) {
        QSettings settings;

        // dns leak enabled
        bool dnsLeak = (settings.value("dnsLeakProtect").toString() == "true");

        smartDns_.enableDnsFeatures(true, dnsLeak, isConnected(), true);
        killTimer( timerVpnFeatures_ );
    }*/
}


bool ConnectWindow::eventFilter(QObject* object, QEvent* event)
{
    if ((object == ui.cbServer && event->type() == QEvent::MouseButtonPress)
        && (state_ == STATE_DISCONNECTED))
    {
        emit serverList();
    }
    return false;
}


void ConnectWindow::onStatisticsChanged(long bytesIn, long bytesOut)
{

    ui.lbDownloadBytes->setText( formatTraficStr(bytesIn) );
    ui.lbUploadBytes->setText( formatTraficStr(bytesOut) );

    if (bFirstBytes)
    {
        prevBytesIn = bytesIn;
        prevBytesOut = bytesOut;
        bFirstBytes = false;
    }
    else
    {
        ui.lbDownloadSpeed->setText( formatTraficStr(bytesIn - prevBytesIn, "/s") );
        ui.lbUploadSpeed->setText( formatTraficStr(bytesOut - prevBytesOut, "/s") );
        prevBytesIn = bytesIn;
        prevBytesOut = bytesOut;
    }
}

void ConnectWindow::onDNSEnabledStateChanged(int state)
{
    QSettings settings;
    settings.setValue("openVpnDNSEnabled", ui.cbEnableSmartDNS->isChecked());
}

void ConnectWindow::saveSettings()
{
	QSettings settings;
    settings.setValue("Server", ui.cbServer->text());
}

void ConnectWindow::loadSettings()
{
	QSettings settings;
	QString server = settings.value("Server").toString();

    if (!servers_.contains(server)) {
        QMap<QString, ServerInfo>::const_iterator it = servers_.constBegin();
        if (it != servers_.constEnd()) {
            ui.cbServer->setText( it.key() );
        } else
            ui.cbServer->setText( "" );
    }else {
        ui.cbServer->setText( server );
    }
}

void ConnectWindow::autoConnect()
{
	onClickConnect();
}

bool ConnectWindow::isConnected()
{
    return (state_ == STATE_CONNECTED);
}

void ConnectWindow::disconnectConnection()
{
    if (state_ != STATE_DISCONNECTED)
    {
        state_ = STATE_DISCONNECTING;
        g_openVPNConnection->disconnect();
    }
}

void ConnectWindow::showLog()
{
    if (logDialog_)
    {
        logDialog_->exec();
    }
}

void ConnectWindow::onClickSmartDns()
{
    smartDNS_OVPNFile_.wait();
    if (!smartDNS_OVPNFile_.success())
    {
        QMessageBox::information(this, QApplication::applicationName(), "Can't download config file for SmartDNS. Please contact support.");
        return;
    }

    mode_ = MODE_SMART_DNS;

    if (state_ == STATE_DISCONNECTED)
    {
        /*
        ui.btnConnect->setStyleSheet("QPushButton { border-image :url(:/MainWindow/Images/btnOff.png); }");
        ui.lbStatusCon->setText(QString("<b>Status:  </b><font color=\"green\">Connecting</font>"));*/

        state_ = STATE_CONNECTING;

        // delete log file
        ALog::Clear();

        ovpnFile_.generate(smartDNS_OVPNFile_.data());

#if defined Q_OS_WIN
        QString ovpnPath = ovpnFile_.path();
#elif defined Q_OS_MAC
        QString ovpnPath = "\'" + ovpnFile_.path() + "\'";
#endif
        g_openVPNConnection->connect(ovpnPath, userName_, password_, "", "");

        mainWindow_->setTrayStatusConnecting();
        ui.cbServer->setEnabled( false );

        ui.tab->setEnabled(false);
        setBarInConnectstatus(true);
    }
    else
    {
        state_ = STATE_DISCONNECTING;
        ui.pbSmartDns->setEnabled(false);
        g_openVPNConnection->disconnect();
    }
}

void ConnectWindow::onRestoreGateway()
{    
    killSwitch_->restoreInternet();
    SAFE_DELETE (pbRestoreGateway)
}

void ConnectWindow::createButtonRestoreGw()
{
    if (!pbRestoreGateway) {
        pbRestoreGateway = new QPushButton(ui.footer);
        connect(pbRestoreGateway, SIGNAL(clicked()), SLOT(onRestoreGateway()));
        pbRestoreGateway->setGeometry(170, 15, 130, 30);
        pbRestoreGateway->setStyleSheet("QPushButton{"
                                   "border-radius: 6px;"
                                   "font: 12pt \"Marid Pro\";"
                                   "color: rgb(255, 255, 255);"
                                   "background-color: red;}"
        );
        pbRestoreGateway->setText("Restore Gateway");
        pbRestoreGateway->setVisible( true );
    }
}

void ConnectWindow::disableVpnFeatures()
{
    /*QSettings settings;
    if (settings.value("vpnKill").toString() == "true")
    {
        killSwitch_->restoreInternet();
    }*/

    //smartDns_.enableDnsFeatures(false, false, false, false);

    //qDebug() <<"disable features OK!";
}

void ConnectWindow::updateIP()
{
    //SAFE_DELETE (getMyVpnIp_)
    if (getMyVpnIp_)
    {
        getMyVpnIp_->terminate();
        getMyVpnIp_->wait();
        getMyVpnIp_->deleteLater();
    }

    getMyVpnIp_ = new GetMyIp(this);
    connect(getMyVpnIp_, SIGNAL(myIpFinished(bool,QString, QString)), SLOT(onMyIpUpdated(bool,QString, QString)));
    getMyVpnIp_->start();
}
