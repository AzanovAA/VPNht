#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include "ui_settingswindow.h"

class WebRTCProtection;

class SettingsWindow : public QWidget
{
	Q_OBJECT

public:
    SettingsWindow(QMenu *settingsMenu, QWidget *parent = 0);
	~SettingsWindow();
	void loadSettings();
	void saveSettings();
    void setEnabledPage(bool enabled);

signals:
    void back();
	void returnToSignUp();

private slots:
    void onSave();
    void onChangeEncryption(int index);
    void onChangeProtocol(int index);

private:
	Ui::SettingsWindow ui;
    WebRTCProtection *webRTCProtection;
};

#endif // SETTINGSWINDOW_H
