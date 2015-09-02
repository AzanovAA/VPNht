#ifndef SIGNUPWINDOW_H
#define SIGNUPWINDOW_H

#include <QWidget>

namespace Ui {
class signupwindow;
}

class signupwindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit signupwindow(QWidget *parent = 0);
    ~signupwindow();
    
private:
    Ui::signupwindow *ui;
};

#endif // SIGNUPWINDOW_H
