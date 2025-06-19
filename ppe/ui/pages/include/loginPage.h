#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QMainWindow>

class QPushButton;
class QStackedWidget;

class LoginPage : public QMainWindow {
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage();

private:
    QStackedWidget *stack;
    QPushButton *startButton;
};

#endif // LOGINPAGE_H
