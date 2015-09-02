#ifndef WEBRTCPROTECTION_H
#define WEBRTCPROTECTION_H

#include <QObject>

class WebRTCProtection : public QObject
{
    Q_OBJECT
public:
    explicit WebRTCProtection(QObject *parent = 0);
    virtual ~WebRTCProtection();

public:
    void enable(bool bEnabled);
    bool isEnable() { return bEnable_; }

private:
    bool bEnable_;
};

#endif // WEBRTCPROTECTION_H
