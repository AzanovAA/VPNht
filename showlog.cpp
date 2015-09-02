#include "showlog.h"
#include <QFile>

ShowLog::ShowLog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

//	QString pathToLog = QApplication::applicationDirPath() + "/log.txt";
//	QFile file(pathToLog);

//	if (file.open(QIODevice::ReadOnly))
//	{
//		QByteArray arr = file.readAll();
//		ui.textBrowser->setPlainText(arr);
//		file.close();
//	}
    setWindowFlags(this->windowFlags() & ~Qt::WindowMaximizeButtonHint
                   & ~Qt::WindowMaximizeButtonHint & ~ Qt::WindowContextHelpButtonHint);
}

ShowLog::~ShowLog()
{

}

void ShowLog::clear()
{
    ui.textBrowser->clear();
}

void ShowLog::onUpdateLog(const QString &text)
{
    ui.textBrowser->append(text);
}
