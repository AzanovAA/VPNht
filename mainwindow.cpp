#include "mainwindow.h"
#include "utils.h"
#include "log.h"
#include "settings.h"
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDesktopWidget>

#ifdef Q_OS_MAC
    #include "Mac/MacApplication.h"
#endif

#ifdef Q_OS_MAC
    #define SERVICE_NAME  "com.aaa.azanov.OVPNHelper"
#elif defined Q_OS_WIN
    #define SERVICE_NAME  "VPNhtOpenVPNService"
#endif

QString ctxMenuStyle = "QMenu { "
        "color: white; "
        "background-color: black; "
        "border: 1px solid transparent; "
        "border-radius: 6px;"
        "font:  11pt \"Marid Pro\";"
        "}"
        "QMenu::item {"
        "padding: 5px;"
        "padding-left: 15px;"
        "padding-right: 15px;"
        "border-bottom: 1px solid white;"
        "}"
        "QMenu::item:last {"
        "padding: 5px;"
        "padding-left: 15px;"
        "padding-right: 15px;"
        "border: none;"
        "}"
        "QMenu::item:selected {"
        "border: none;"
        "background-color: white; "
        "color: black; "
        "}";

bool needExit = false;

OpenVPNConnectorQt *g_openVPNConnection = NULL;


MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags), loginWindow_(NULL), connectWindow_(NULL), serverListWindow_(0),
      timer_(NULL), settingsMenu_(0), connected_(0), getServers_(NULL)
{
	QCoreApplication::setOrganizationName("CPPP");
    QCoreApplication::setApplicationName("VPNht");

	ui.setupUi(this);
    setWindowTitle("VPN.ht " + QString(VERSION));

    g_openVPNConnection = new OpenVPNConnectorQt(this);
    if (!g_openVPNConnection->installHelper(SERVICE_NAME))
    {
        QMessageBox::information(this,
                                 "VPNht", "Failed to install helper for program. Please contact support.");
        QTimer::singleShot(1, this, SLOT(close()));
        return;
    }

#if defined Q_OS_MAC
    //setAttribute(Qt::WA_QuitOnClose, false);
#endif

    trayIcon_ = new QSystemTrayIcon(this);       

    trayMenu_ = new QMenu(this);
    actConnect_ = new QAction("Connect", this);
    actDisconnect_ = new QAction("Disconnect", this);
    actHelp_ = new QAction("Help", this);
    actLiveChat_ = new QAction("Live chat", this);
    actExit_ = new QAction("Quit", this);
    trayMenu_->addAction(actConnect_);
    trayMenu_->addAction(actDisconnect_);
    trayMenu_->addSeparator();
    trayMenu_->addAction(actHelp_);
    trayMenu_->addAction(actLiveChat_);
    trayMenu_->addAction(actExit_);
    trayIcon_->setContextMenu(trayMenu_);
    trayIcon_->hide();    

    setTrayStatusDisconnected(false);

    settingsMenu_ = new QMenu(this);
    settingsMenu_->addAction("Settings", this, SLOT(onSettings()));
    settingsMenu_->addAction("Status", this, SLOT(onStatus()));
    settingsMenu_->addAction("Help", this, SLOT(onHelp()));
    settingsMenu_->addAction("Proxy", this, SLOT(onProxy()));
    settingsMenu_->addAction("Show logs", this, SLOT(onShowLogs()));
    settingsMenu_->addAction("Logout", this, SLOT(onReturnToSignUp()));
    settingsMenu_->setStyleSheet(ctxMenuStyle);    

    loginWindow_ = new LoginWindow(this);
	ui.verticalLayout->addWidget(loginWindow_);
	loginWindow_->loadSettings();
    loginWindow_->hide();
	connect(loginWindow_, SIGNAL(signedIn(QString, QString)), SLOT(onSignIn(QString, QString)));

    waitWindow_ = new WaitWindow(this);
    ui.verticalLayout->addWidget(waitWindow_);

    settingsWindow_ = new SettingsWindow(settingsMenu_, this);
    connect(settingsWindow_, SIGNAL(back()), SLOT(onBack()));
	ui.verticalLayout->addWidget(settingsWindow_);
    settingsWindow_->hide();

    connectWindow_ = new ConnectWindow(settingsMenu_, this);
    connect(connectWindow_, SIGNAL(serverList()), SLOT(onServerList()));
    connect(connectWindow_, SIGNAL(connected()), SLOT(onConnected()));
        ui.verticalLayout->addWidget(connectWindow_);

    ui.verticalLayout->addWidget(connectWindow_);
	connectWindow_->hide();

    serverListWindow_ = new ServerListWindow(this);
    connect(serverListWindow_, SIGNAL(serverSelected(const QString &)),
            this, SLOT( onChangeServer(const QString &) ));
    ui.verticalLayout->addWidget(serverListWindow_);
    serverListWindow_->hide();

    proxySettingsWindow_ = new ProxySettingsWindow(settingsMenu_, this);
    connect(proxySettingsWindow_, SIGNAL(back()), SLOT(onBack()));
    ui.verticalLayout->addWidget(proxySettingsWindow_);
    proxySettingsWindow_->hide();


    // tray action
    connect(actConnect_, SIGNAL(triggered()), connectWindow_, SLOT(onClickConnect()));
    connect(actDisconnect_, SIGNAL(triggered()), connectWindow_, SLOT(onClickConnect()));
    connect(actHelp_, SIGNAL(triggered()), SLOT(onActHelp()));
    connect(actLiveChat_, SIGNAL(triggered()), SLOT(onLiveChat()));
    connect(actExit_, SIGNAL(triggered()), SLOT(onExit()));


#if defined Q_OS_WIN
    connect(trayIcon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            SLOT( onTrayActivated(QSystemTrayIcon::ActivationReason) ));
#endif

    QSettings settings;
    if (settings.value("savePass", "true").toString() == "true")
    {
		QString username = settings.value("login", "").toString();
		QString password = settings.value("password", "").toString();
		if (!username.isEmpty() && !password.isEmpty())
        {
            adjustSize();
            setFixedSize(size());
            skipLoginScreen(username, password);
		}
		else
		{
            waitWindow_->hide();
            loginWindow_->show();
            adjustSize();
            setFixedSize(size());
		}
    }
    else
    {
        waitWindow_->hide();
        loginWindow_->show();
        adjustSize();
        setFixedSize(size());
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::onSignIn(QString login, QString password)
{    
    loginWindow_->hide();
    getServers_ = loginWindow_->getServers();
    connectWindow_->setServers(getServers_->servers());
    connectWindow_->setAcountData(login, password);
    connectWindow_->show();
    adjustSize();
    //setFixedSize(size());

    placeWindowToCenter();

    QSettings settings;
    QString autoConnectOnStart = settings.value("autoConnectOnStart", "false").toString();
    if (autoConnectOnStart == "true")
    {
        connectWindow_->autoConnect();
    }
}


void MainWindow::onStatus()
{
    settingsWindow_->hide();
    proxySettingsWindow_->hide();
    connectWindow_->show();
    adjustSize();
    setFixedSize(size());
}

void MainWindow::onHelp()
{
    QString link = "https://vpn.ht/contact";
    QDesktopServices::openUrl(QUrl(link));
}

void MainWindow::onProxy()
{
    connectWindow_->hide();
    settingsWindow_->hide();
    proxySettingsWindow_->loadSettings();
    proxySettingsWindow_->show();
    adjustSize();
    setFixedSize(size());
}

void MainWindow::onShowLogs()
{
    connectWindow_->showLog();
}

void MainWindow::onSettings()
{
	connectWindow_->saveSettings();
    //setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
	connectWindow_->hide();
    proxySettingsWindow_->hide();
	settingsWindow_->loadSettings();
    settingsWindow_->setEnabledPage( !connected_);
	settingsWindow_->show();
	adjustSize();
	setFixedSize(size());
}

void MainWindow::onServerList()
{
    connectWindow_->saveSettings();
    connectWindow_->hide();
    proxySettingsWindow_->hide();
    settingsWindow_->hide();
    serverListWindow_->setServers( getServers_->servers() );
    serverListWindow_->show();
    serverListWindow_->adjustSize();
    adjustSize();
    setFixedSize(size());
}

void MainWindow::onChangeServer(const QString &server)
{
    serverListWindow_->hide();
    settingsWindow_->hide();
    proxySettingsWindow_->hide();
    connectWindow_->show();
    connectWindow_->onServerChanged(server);
    adjustSize();
    setFixedSize(size());
}

void MainWindow::onBack()
{
    //setFixedSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    proxySettingsWindow_->hide();
    proxySettingsWindow_->saveSettings();
    settingsWindow_->hide();
    connectWindow_->show();
    adjustSize();
    setFixedSize(size());
}

void MainWindow::onReturnToSignUp()
{
    if (connectWindow_->isConnected()) {
        int res = QMessageBox::question(this, "Logout", "VPN connection is active. Disconnect and logout?",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (res == QMessageBox::No) {

            return;
        }
    }
    connectWindow_->disconnectConnection();
    connectWindow_->hide();
	settingsWindow_->hide();
	settingsWindow_->saveSettings();
    proxySettingsWindow_->hide();
	loginWindow_->loadSettings();
	loginWindow_->show();    
	adjustSize();
	setFixedSize(size());

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    /*if (!needExit) {
        showMinimized();
        event->ignore();
        return;
    }*/

    if (connectWindow_)
    {

        if (connectWindow_->isConnected()) {
            show();
            int res = QMessageBox::question(this, "Exit", "VPN connection is active. Disconnect and exit?",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (res == QMessageBox::No) {
                event->ignore();
                return;
            }
        }

        if (settingsWindow_->isVisible())
        {
            settingsWindow_->saveSettings();
        }
        if (connectWindow_->isVisible())
        {
            connectWindow_->saveSettings();
        }
    }
    QMainWindow::closeEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{    
    if (event->type() == QEvent::WindowStateChange) {
        if (windowState() & Qt::WindowMinimized) {
            trayIcon_->show();
            QTimer::singleShot(0, this, SLOT(hide()));
            event->ignore();
            return;
        }else {
            trayIcon_->hide();
            QMainWindow::changeEvent(event);
            return;
        }
    }

    QMainWindow::changeEvent(event);
}


void MainWindow::onGetServersFinished(bool bSuccess, QString errorStr, bool bConfigChanged)
{
    if (bSuccess)
    {
        waitWindow_->hide();
        connectWindow_->setServers(getServers_->servers());
        connectWindow_->show();
        adjustSize();
        setFixedSize(size());

        placeWindowToCenter();

        QSettings settings;
        QString autoConnectOnStart = settings.value("autoConnectOnStart", "false").toString();
        if (autoConnectOnStart == "true")
        {
            connectWindow_->autoConnect();
        }
    }
    else
    {
        onAuthenticationError();
    }
}

void MainWindow::onAuthenticationError()
{
    waitWindow_->hide();
    loginWindow_->show();
    adjustSize();
    setFixedSize(size());
    placeWindowToCenter();
    QMessageBox::information(this, QApplication::applicationName(), "User authentication error.");
}

void MainWindow::skipLoginScreen(const QString &username, const QString &password)
{
    connectWindow_->setAcountData(username, password);
    getServers_ = new GetServers(this);
    getServers_->setLoginData(username, password);
    connect(getServers_, SIGNAL(updateFinished(bool, QString, bool)), SLOT(onGetServersFinished(bool, QString, bool)));
    connect(getServers_, SIGNAL(authenticationError()), SLOT(onAuthenticationError()));
    getServers_->start();
}

void MainWindow::placeWindowToCenter()
{
    QDesktopWidget *desktopWidget = QApplication::desktop();
    QRect rcDesktop = desktopWidget->availableGeometry();

    QRect rcWindow = this->geometry();

    rcWindow.moveCenter(rcDesktop.center());


    this->setGeometry(rcWindow);
}

void MainWindow::setTrayStatusConnected()
{  
    connected_ = true;

    actConnect_->setEnabled( false );
    actDisconnect_->setEnabled( true );
    trayIcon_->setIcon(QIcon(":/MainWindow/Images/vpnlogo.ico"));
//    trayIcon_->setToolTip("Connection is secured");

//    QString msg = tr("Your internet traffic is encrypted and secured now!");
}

void MainWindow::setTrayStatusDisconnected(bool bShowMessage)
{
    connected_ = false;
    actConnect_->setEnabled( true );
    actDisconnect_->setEnabled( false );
    trayIcon_->setIcon(QIcon(":/MainWindow/Images/vpnlogo.ico"));
//    trayIcon_->setToolTip("Connection is not secured");

//    if (!trayIcon_->isVisible())
//    {
//        trayIcon_->show();
//    }

//    if (bShowMessage)
//    {
//        QString msg = tr("Your internet traffic is not secured anymore");
//        ///trayIcon_->showMessage(tr("VPNLayer"), msg);
//    }
}

void MainWindow::setTrayStatusConnecting()
{
    /*SAFE_DELETE(timer_);
    timer_ = new QTimer(this);
    timer_->setInterval(500);
    connect(timer_, SIGNAL(timeout()), SLOT(onTimer()));
    state_ = 0;
    timer_->start();*/
}

void MainWindow::onShow()
{
    trayIcon_->hide();
    show();
    setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
    activateWindow();
#ifdef Q_OS_MAC
    MacApplication *macApp = (MacApplication *)QApplication::instance();
    macApp->activateIgnoring();
#endif
    raise();   
}

void MainWindow::onActHelp()
{
    QString link = "https://vpn.ht/documentation";
    QDesktopServices::openUrl(QUrl(link));
}


void MainWindow::onLiveChat()
{
    QString link = "https://vpn.ht/#open-chat";
    QDesktopServices::openUrl(QUrl(link));
}


void MainWindow::onExit()
{
    needExit = true;
    trayIcon_->hide();
    this->showNormal();
    this->activateWindow();

    close();
    //QApplication::instance()->quit();
}

void MainWindow::onConnected()
{
//    trayIcon_->show();
//    trayIcon_->showMessage("", "VPNht connected", QSystemTrayIcon::Information, 3000);
//    hide();
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
        case QSystemTrayIcon::DoubleClick:
        case QSystemTrayIcon::Trigger: {
            this->showNormal();
            this->activateWindow();
        }break;

        default:
        break;
    }
}

