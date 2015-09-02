#ifndef HELLOWINDOW_H
#define HELLOWINDOW_H

#include <QWidget>

namespace Ui {
class hellowindow;
}

class hellowindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit hellowindow(QWidget *parent = 0);
    ~hellowindow();
    
private:
    Ui::hellowindow *ui;
};

#endif // HELLOWINDOW_H
