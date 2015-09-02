#pragma once

#include <QWidget>
#include <QMap>
#include "getservers.h"

class QStandardItem;
class QStandardItemModel;
class PingWrapper;

namespace Ui {
class ServerListWindow;
}

class ServerListWindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit ServerListWindow(QWidget *parent = 0);
    ~ServerListWindow();
    void setServers(QVector<ServerInfo> );
    ServerInfo getSelectedServer();

signals:
    void serverSelected(const QString &server   );

private slots:
    void selServer();
    void onPingFinished(bool,double,QString);
    void onPingTimerTimeout();

private:
    Ui::ServerListWindow *ui;
    QStandardItemModel* dnsModel_;
    QVector<ServerInfo> servers_;
    ServerInfo  selServer_;
    QMap<QString, PingWrapper*> pingMap;
    QMap<QString, QStandardItem*> itemMap;
    QTimer* pingTimer;

    void updateTable();
};
