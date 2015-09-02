#ifndef SHOWLOG_H
#define SHOWLOG_H

#include <QDialog>
#include "ui_showlog.h"

class ShowLog : public QDialog
{
	Q_OBJECT

public:
	ShowLog(QWidget *parent = 0);
	~ShowLog();
    void clear();

public slots:
    void onUpdateLog(const QString&);

private:
	Ui::ShowLog ui;
};

#endif // SHOWLOG_H
