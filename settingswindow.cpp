#include "settingswindow.h"
#include <QSettings>
#include <QDir>
#include <QListView>
#include <QDebug>
#include "utils.h"
#include "webrtcprotection.h"

#if defined Q_OS_MAC
    #include <CoreFoundation/CoreFoundation.h>
    #include <CoreServices/CoreServices.h>
#endif

SettingsWindow::SettingsWindow(QMenu *settingsMenu, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    ui.btnSettings->setMenu( settingsMenu );
    connect(ui.pbSave, SIGNAL(clicked()), SLOT(onSave()));

    webRTCProtection = new WebRTCProtection(this);

    // fill encryption
    for (int i=0; gVpnEncryption[i].id_ != -1; i++) {
        ui.cbEncryption->addItem(gVpnEncryption[i].label_, gVpnEncryption[i].id_);
    }    

    // fill protocol
    for (int i=0; gVpnProtocol[i].id_ != -1; i++) {
        ui.cbProtocol->addItem(gVpnProtocol[i].label_, gVpnProtocol[i].id_);
    }       

    loadSettings();

    connect(ui.cbEncryption, SIGNAL(currentIndexChanged(int)), SLOT(onChangeEncryption(int)));
    connect(ui.cbProtocol, SIGNAL(currentIndexChanged(int)), SLOT(onChangeProtocol(int)));
}

SettingsWindow::~SettingsWindow()
{
    SAFE_DELETE(webRTCProtection);
}

void SettingsWindow::onSave()
{
    saveSettings();

	emit back();
}

void SettingsWindow::onChangeEncryption(int index)
{
    if (index == -1)
        return;

    ui.cbProtocol->clear();
    ui.cbProtocol->setCurrentIndex(-1);
    int enryptID = ui.cbEncryption->itemData(index).toInt();

    // fill protocol
    for (int i=0; gVpnProtocol[i].id_ != -1; i++) {
        if (getAvailablePort(gVpnProtocol[i].id_, enryptID).id_ != -1) {
            ui.cbProtocol->addItem(gVpnProtocol[i].label_, gVpnProtocol[i].id_);
        }
    }
}

void SettingsWindow::onChangeProtocol(int index)
{
//    if (index == -1)
//        return;

//    ui.cbProtocol->clear();
//    ui.cbProtocol->setCurrentIndex(-1);
//    int protocolID = ui.cbProtocol->itemData(index).toInt();
//    qDebug() <<"onChangeProtocol: " <<protocolID;

//    // fill protocol
//    for (int i=0; gVpnProtocol[i].id_ != -1; i++) {
//        if (getAvailablePort(gVpnProtocol[i].id_, enryptID).id_ != -1) {
//            qDebug() <<gVpnProtocol[i].name_ <<gVpnProtocol[i].id_;
//            ui.cbProtocol->addItem(gVpnProtocol[i].label_, gVpnProtocol[i].id_);
//        }
//    }
}

void SettingsWindow::setEnabledPage(bool enabled)
{
    ui.cbKillVpn->setEnabled( enabled );
    ui.cbDnsLeakProtect->setEnabled( enabled);
    ui.cbAutoconnectOnStart->setEnabled( enabled);
    ui.cbLaunchOnStart->setEnabled( enabled);
    ui.cbProtocol->setEnabled( enabled);
    ui.cbEncryption->setEnabled( enabled);
    ui.pbSave->setEnabled( enabled);
    ui.cbWebRTCProtection->setEnabled( enabled);
}

void SettingsWindow::saveSettings()
{
	QSettings settings;
    settings.setValue("vpnKill", ui.cbKillVpn->isChecked());
    settings.setValue("dnsLeakProtect", ui.cbDnsLeakProtect->isChecked());
    settings.setValue("autoConnectOnStart", ui.cbAutoconnectOnStart->isChecked());
    settings.setValue("launchOnStart", ui.cbLaunchOnStart->isChecked());
    settings.setValue("vpnProtocol", ui.cbProtocol->currentText());
    settings.setValue("vpnEncryption", ui.cbEncryption->currentText());

    webRTCProtection->enable(ui.cbWebRTCProtection->isChecked());
    settings.setValue("webRTCProtection", ui.cbWebRTCProtection->isChecked());

#if defined Q_OS_WIN

    if (ui.cbLaunchOnStart->isChecked())
    {
        QSettings settingsRun("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        QString exePath = QDir::toNativeSeparators(QApplication::applicationFilePath());
        settingsRun.setValue("ProxyRack", exePath);
    }
    else
    {
        QSettings settingsRun("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        settingsRun.remove("ProxyRack");
    }
#elif defined Q_OS_MAC

    bool bHasAutoRunItem  = false;
    bool bNeedAutoRunItem = ui.cbLaunchOnStart->isChecked();
    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());

    LSSharedFileListRef loginItems;
    loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL );

    UInt32 seedValue;
    CFArrayRef loginArray = LSSharedFileListCopySnapshot(loginItems, &seedValue);

    LSSharedFileListItemRef loginItem = NULL;
    for (CFIndex I = 0, SZ = CFArrayGetCount(loginArray); I < SZ; ++I)
    {
        loginItem = (LSSharedFileListItemRef) CFArrayGetValueAtIndex(loginArray, I);

        CFURLRef itemUrlRef;
        if (noErr == LSSharedFileListItemResolve(loginItem, 0, &itemUrlRef, NULL))
        {
            if (CFStringCompare( CFURLGetString(itemUrlRef), CFURLGetString(appUrlRef) , 0) == 0)
            {
                if (!bNeedAutoRunItem)
                    LSSharedFileListItemRemove(loginItems, loginItem);
                else
                    bHasAutoRunItem = true;
                break;
            }
        }
    }
    if (!bHasAutoRunItem && bNeedAutoRunItem)
    {
        LSSharedFileListInsertItemURL(loginItems , kLSSharedFileListItemLast, NULL, NULL, appUrlRef, NULL, NULL );
    }

    if (loginArray) CFRelease(loginArray);

    CFRelease(appUrlRef);
    CFRelease(loginItems);

#endif

}

void SettingsWindow::loadSettings()
{
	QSettings settings;

    ui.cbLaunchOnStart->setChecked(settings.value("launchOnStart", "false").toString() == "true");
	ui.cbAutoconnectOnStart->setChecked(settings.value("autoConnectOnStart", "false").toString() == "true");
    ui.cbDnsLeakProtect->setChecked(settings.value("dnsLeakProtect", "false").toString() == "true");
    ui.cbKillVpn->setChecked( settings.value("vpnKill", "false").toString() == "true" );    

    bool bWebRTCEnable = settings.value("webRTCProtection", false).toBool();
    ui.cbWebRTCProtection->setChecked(bWebRTCEnable);
    webRTCProtection->enable(bWebRTCEnable);

    for (int i=0; gVpnEncryption[i].id_ != -1; i++) {
        if (gVpnEncryption[i].label_ == settings.value("vpnEncryption").toString()) {
            ui.cbEncryption->setCurrentIndex(i);
            onChangeEncryption(i);

            for (int j=0; j < ui.cbProtocol->count(); j++) {
                qDebug() <<ui.cbProtocol->itemText(j);
                if (ui.cbProtocol->itemText(j) == settings.value("vpnProtocol").toString()) {
                    ui.cbProtocol->setCurrentIndex(j);
                    break;
                }
            }
            return;
        }
    }

    ui.cbEncryption->setCurrentIndex(1);
    onChangeEncryption(1);
}
