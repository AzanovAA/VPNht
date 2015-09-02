#include "serverlistwindow.h"
#include "ui_serverlistwindow.h"
#include <QStandardItemModel>
#include <QDebug>
#include <QTimer>
#include "pingwrapper.h"

ServerListWindow::ServerListWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerListWindow),
    dnsModel_(0)
{
    ui->setupUi(this);

#if defined Q_OS_MAC
    ui->treeView->setAttribute(Qt::WA_MacShowFocusRect, 0);
#endif

    dnsModel_ = new QStandardItemModel();
    ui->treeView->setModel(dnsModel_);
    connect(ui->treeView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(selServer()));

    pingTimer = new QTimer(this);
    pingTimer->setInterval(100);
    connect(pingTimer, SIGNAL(timeout()), SLOT(onPingTimerTimeout()));
    //pingTimer->start();
    //updateTable();
}

ServerListWindow::~ServerListWindow()
{
    delete ui;
    delete pingTimer;

    foreach (PingWrapper* pw, pingMap)
    {
        if (pw->isRunning())
        {
            pw->terminate();
            pw->wait();
        }
        pw->deleteLater();
    }
}

void ServerListWindow::setServers(QVector<ServerInfo> list)
{
    servers_ = list;
    updateTable();
}

void ServerListWindow::selServer()
{
    pingTimer->stop();
    int idx = ui->treeView->currentIndex().row();
    selServer_ = servers_[idx];
    emit serverSelected( selServer_.country_ );
}

void ServerListWindow::updateTable()
{
    itemMap.clear();
    dnsModel_->clear();
    dnsModel_->setColumnCount( 2 );

    dnsModel_->setHeaderData(0, Qt::Horizontal, "LOCATION" );
    dnsModel_->setHeaderData(1, Qt::Horizontal, "PING" );    

    int row = 0;

    foreach(ServerInfo si, servers_)
    {
        QStandardItem* i = new QStandardItem( si.country_ );
        dnsModel_->appendRow( i );

        itemMap.insert(si.ip_, i);

        dnsModel_->setData(dnsModel_->index(row, 1), "n\\a");
        row++;        
    }

    ui->treeView->header()->resizeSection(0, 240);
    ui->treeView->header()->resizeSection(1, 120);

    foreach(ServerInfo si, servers_)
    {
        if (pingMap.contains(si.ip_))
            continue;

        PingWrapper *pingwrapper = new PingWrapper(si.ip_);
        connect(pingwrapper, SIGNAL(pingFinished(bool,double,QString)),
                SLOT(onPingFinished(bool,double,QString)));
        pingwrapper->moveToThread(pingwrapper);
        pingwrapper->start();

        pingMap.insert(si.ip_, pingwrapper);
    }

    //restart ping host
    pingTimer->start();
}

void ServerListWindow::onPingFinished(bool success, double time, QString host)
{
    qDebug() <<success <<time <<host;
    if (itemMap.contains(host)) {
        QStandardItem* i = itemMap.value(host);

        if (success)
            dnsModel_->setData(dnsModel_->index( i->row(), 1),  QString("%1 ms").arg(time) );
        else
            dnsModel_->setData(dnsModel_->index( i->row(), 1), "n\\a");
    }
}

void ServerListWindow::onPingTimerTimeout()
{
    foreach (PingWrapper* pw, pingMap) {
        if (pw->isFinished()) {
            pw->start();
        }
    }

    pingTimer->setInterval(3000);
    pingTimer->start();
}
