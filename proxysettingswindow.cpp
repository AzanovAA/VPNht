#include "proxysettingswindow.h"
#include "ui_proxysettingswindow.h"

#include <QSettings>

ProxySettingsWindow::ProxySettingsWindow(QMenu *settingsMenu, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::proxysettingswindow),
    proxyIsUsed_(0)
{
    ui->setupUi(this);

    ui->btnSettings->setMenu( settingsMenu );
    connect(ui->pbSave, SIGNAL(clicked()), SLOT(onSave()));
    connect(ui->pbSwitchOn, SIGNAL(clicked()), SLOT(onSwitchProxy()));
}

ProxySettingsWindow::~ProxySettingsWindow()
{
    delete ui;
}

void ProxySettingsWindow::setEnabledPage(bool enabled)
{
    //ui->pbSwitchOn->setEnabled( enabled);

    if (!enabled) {
        ui->pbSwitchOn->setStyleSheet("QPushButton {"
            "border-image :url(:/MainWindow/Images/btnOff.png);}");
    } else {
        ui->pbSwitchOn->setStyleSheet("QPushButton {"
            "border-image :url(:/MainWindow/Images/btnOn.png);}");
    }

    ui->leProxyUrl->setEnabled( enabled );
    ui->leProxyPort->setEnabled( enabled );
    ui->leProxyUsername->setEnabled( enabled );
    ui->leProxyPassword->setEnabled( enabled );
}

void ProxySettingsWindow::onSwitchProxy()
{
    proxyIsUsed_ = !proxyIsUsed_;
    setEnabledPage( proxyIsUsed_ );
    emit onChangeState(proxyIsUsed_);
}

void ProxySettingsWindow::onSave()
{
    saveSettings();

    emit onChangeSettings(proxyIsUsed_);
    emit back();
}

void ProxySettingsWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("proxyUrl", ui->leProxyUrl->text());
    settings.setValue("proxyPort", ui->leProxyPort->text());
    settings.setValue("proxyUser", ui->leProxyUsername->text());
    settings.setValue("proxyPassword", ui->leProxyPassword->text());
    settings.setValue("proxyIsOn", proxyIsUsed_);
}

void ProxySettingsWindow::loadSettings()
{
    QSettings settings;

    proxyIsUsed_ = settings.value("proxyIsOn", "false").toString() == "true";
    setEnabledPage(proxyIsUsed_);

    ui->leProxyUrl->setText( settings.value("proxyUrl", "").toString() );
    ui->leProxyPort->setText( settings.value("proxyPort", "").toString() );
    ui->leProxyUsername->setText( settings.value("proxyUser", "").toString() );
    ui->leProxyPassword->setText( settings.value("proxyPassword", "").toString() );
}
