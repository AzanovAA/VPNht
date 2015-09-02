#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include "ui_loginwindow.h"

class GetServers;

class LoginWindow : public QWidget
{
	Q_OBJECT

public:
    LoginWindow(QWidget *parent = 0);
	~LoginWindow();
    void loadSettings();

    GetServers* getServers() { return getServers_; }

signals:
	void signedIn(QString login, QString password);

private slots:
    void onSignIn();
    void onSignUp();
    void onForgotLink(const QString &str);

    void onGetServerFinished(bool bSuccess, QString errorStr, bool bConfigChanged);
    void onAuthenticationError();

private:
	Ui::LoginWindow ui;
    GetServers *getServers_;
};

#endif // LOGINWINDOW_H
