#include <QThread>
#include <QProcess>

class QNetworkReply;

class PingWrapper : public QThread
{
    Q_OBJECT

public:
    PingWrapper(QString host, QObject *parent = NULL);

signals:
    void pingFinished(bool bSuccess, double timems, QString host);

protected:
    virtual void run();

private:
    QString host_;
};
