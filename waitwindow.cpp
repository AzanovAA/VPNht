#include "waitwindow.h"
#include "ui_waitwindow.h"

WaitWindow::WaitWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaitWindow)
{
    ui->setupUi(this);
}

WaitWindow::~WaitWindow()
{
    delete ui;
}
