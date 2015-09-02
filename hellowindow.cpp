#include "hellowindow.h"
#include "ui_hellowindow.h"

hellowindow::hellowindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::hellowindow)
{
    ui->setupUi(this);
}

hellowindow::~hellowindow()
{
    delete ui;
}
