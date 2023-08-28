/*
登录界面：
    1.登录
    2.注册
    3.退出
*/

#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include "database.h"
#include <QString>
#include <QDebug>
#include <QMessageBox>
#include "registerwidget.h"
#include "databasewidget.h"
#include "managerwidget.h"
#include "FramelessWindow/framelesswindow.h"
#include "mainwindow.h"
#include <QDateTime>

namespace Ui {
class login;
}

class login : public QWidget
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login();

private slots:
    void on_LoginButton_clicked();

    void on_RegisterButton_clicked();

    void on_CloseButton_clicked();

private:
    Ui::login *ui;
    Database myDatabase;
};

#endif // LOGIN_H
