#include "mainwindow.h"
#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>
#include <QNetworkConfigurationManager>
#include <QTime>

#if defined Q_OS_WIN
#include "QtSingleApplication/qtsingleapplication.h"
#elif defined Q_OS_MAC
#include "Mac/MacApplication.h"
#endif

int main(int argc, char *argv[])
{
#if defined Q_OS_WIN
    QtSingleApplication a(argc, argv);
#elif defined Q_OS_MAC
    MacApplication a(argc, argv);
#endif

    if (a.isRunning())
        return a.sendMessage(QObject::tr("Can't start more than one instance of the application."));

	MainWindow w;
    w.show();

    return a.exec();
}
