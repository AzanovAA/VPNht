#include "signupwindow.h"
#include "ui_signupwindow.h"

signupwindow::signupwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::signupwindow)
{
    ui->setupUi(this);
}

signupwindow::~signupwindow()
{
    delete ui;
}
