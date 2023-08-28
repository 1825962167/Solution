#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>
#include "database.h"
#include <QString>
#include <QDebug>
#include <QMessageBox>
#include "login.h"

namespace Ui {
class RegisterWidget;
}

class RegisterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget *parent = nullptr);
    ~RegisterWidget();

private slots:
    void on_registerButton_clicked();

    void on_backButton_clicked();

private:
    Ui::RegisterWidget *ui;
    Database MyDatabase;
};

#endif // REGISTERWIDGET_H
