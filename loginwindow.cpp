#include "loginwindow.h"
#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QUrl>
#include <QtNetwork>
#include <QSslConfiguration>
#include "getservers.h"

LoginWindow::LoginWindow(QWidget *parent)
	: QWidget(parent)
{
    ui.setupUi(this);

#if defined Q_OS_MAC
    ui.edLogin->setAttribute(Qt::WA_MacShowFocusRect, 0);
    ui.edPassword->setAttribute(Qt::WA_MacShowFocusRect, 0);
#endif

    connect(ui.pbSignIn, SIGNAL(clicked()), SLOT(onSignIn()));
    connect(ui.pbSignUp, SIGNAL(clicked()), SLOT(onSignUp()));
    connect(ui.lbForgotPass, SIGNAL(linkActivated(const QString &)),
            SLOT(onForgotLink(const QString &)));

    getServers_ = new GetServers(parent);
    connect(getServers_, SIGNAL(updateFinished(bool, QString, bool)), SLOT(onGetServerFinished(bool, QString, bool)));
    connect(getServers_, SIGNAL(authenticationError()), SLOT(onAuthenticationError()));
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::onSignIn()
{
    if (ui.edLogin->text().isEmpty() || ui.edPassword->text().isEmpty())
	{
		QMessageBox::information(this, QApplication::applicationName(), tr("You must enter username and password to sign in"));
        return;
	}

    QApplication::setOverrideCursor(Qt::WaitCursor);
    setEnabled(false);

    getServers_->setLoginData(ui.edLogin->text(), ui.edPassword->text());
    getServers_->start();
}

void LoginWindow::onSignUp()
{
    //TODO: заменить линк
    QString link = "https://www.vpn.ht/signup";
	QDesktopServices::openUrl(QUrl(link));
}

void LoginWindow::onForgotLink(const QString &str)
{
    //TODO: заменить линк
    QString link = "https://www.vpn.ht/signup";
    QDesktopServices::openUrl(QUrl(link));
}

void LoginWindow::onGetServerFinished(bool bSuccess, QString errorStr, bool bConfigChanged)
{
    setEnabled(true);
    QApplication::restoreOverrideCursor();
    if (!bSuccess)
    {
        QMessageBox::information(this, QApplication::applicationName(), "Error during get list of servers.");
    }
    else
    {
        QSettings settings;
        settings.setValue("login", ui.edLogin->text());
        settings.setValue("password", ui.edPassword->text());
        settings.setValue("savePass", ui.cbSavePassword->isChecked());

        emit signedIn(ui.edLogin->text(), ui.edPassword->text());
    }
}

void LoginWindow::onAuthenticationError()
{
    setEnabled(true);
    QApplication::restoreOverrideCursor();
    QMessageBox::information(this, QApplication::applicationName(), "User authentication error.");
}

void LoginWindow::loadSettings()
{
	QSettings settings;
	ui.edLogin->setText(settings.value("login", "").toString());
	ui.edPassword->setText(settings.value("password", "").toString());
    ui.cbSavePassword->setChecked(settings.value("savePass", "true").toString() == "true");
}
