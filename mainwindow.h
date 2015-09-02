#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "loginwindow.h"
#include "connectwindow.h"
#include "settingswindow.h"
#include "proxysettingswindow.h"
#include "serverlistwindow.h"
#include "getservers.h"
#include "waitwindow.h"
#include <QDebug>
#include <QSystemTrayIcon>
#include <QMenu>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
    MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~MainWindow();

    void setTrayStatusConnected();
    void setTrayStatusDisconnected(bool bShowMessage = true);
    void setTrayStatusConnecting();

protected:
	void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);

private slots:
	void onSignIn(QString login, QString password);
	void onSettings();
    void onBack();
	void onReturnToSignUp();
    void onShow();
    void onExit();
    void onStatus();
    void onConnected();
    void onHelp();
    void onActHelp();
    void onLiveChat();
    void onProxy();
    void onShowLogs();
    void onServerList();
    void onChangeServer(const QString &);
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onGetServersFinished(bool bSuccess, QString errorStr, bool bConfigChanged);
    void onAuthenticationError();

private:
	Ui::MainWindowClass ui;
	LoginWindow *loginWindow_;
	ConnectWindow *connectWindow_;
	SettingsWindow *settingsWindow_;
    ProxySettingsWindow *proxySettingsWindow_;
    ServerListWindow *serverListWindow_;
    WaitWindow *waitWindow_;
    GetServers *getServers_;
    bool connected_;
    QSystemTrayIcon *trayIcon_;
    QMenu   *trayMenu_;
    QAction *actConnect_, *actDisconnect_, *actHelp_, *actLiveChat_, *actExit_;
    QTimer *timer_;
    int state_;
    QMenu *settingsMenu_;

    void skipLoginScreen(const QString &username, const QString &password);
    void placeWindowToCenter();
};

#endif // MAINWINDOW_H
