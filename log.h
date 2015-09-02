//---------------------------------------------------------------------------
#pragma once
#include <QMutex>

#define SAFE_DELETE(x)  if (x) { delete x; x = NULL; }

class QWidget;

class ALog
{
public:
    static QWidget *parent_;
    static void Out(QString str);
    static void Clear();
    static QMutex mutex_;
};
//---------------------------------------------------------------------------
