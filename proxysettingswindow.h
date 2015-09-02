#ifndef PROXYSETTINGSWINDOW_H
#define PROXYSETTINGSWINDOW_H

#include <QWidget>
#include <QMenu>

namespace Ui {
class proxysettingswindow;
}

class ProxySettingsWindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit ProxySettingsWindow(QMenu *settingsMenu, QWidget *parent = 0);
    ~ProxySettingsWindow();
    void loadSettings();
    void saveSettings();
    void setEnabledPage(bool enabled);

signals:
    void onChangeState(bool enabled);
    void onChangeSettings(bool enabled);
    void back();

private slots:
    void onSwitchProxy();
    void onSave();
    
private:
    Ui::proxysettingswindow *ui;
    bool proxyIsUsed_;
};

#endif // PROXYSETTINGSWINDOW_H
