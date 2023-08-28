#ifndef OPERATORDATABASEWIDGET_H
#define OPERATORDATABASEWIDGET_H

#include <QMainWindow>
#include "databasewidget.h"

namespace Ui {
class OperatorDatabaseWidget;
}

class OperatorDatabaseWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit OperatorDatabaseWidget(QWidget *parent = nullptr);
    ~OperatorDatabaseWidget();

private slots:
    void on_actiondaochu_triggered();   //导出action

    void on_actiondayin_triggered();    //打印action

signals:
    void export_table();
    void print_table();

private:
    Ui::OperatorDatabaseWidget *ui;
    DatabaseWidget *databaseWidget;
};

#endif // OPERATORDATABASEWIDGET_H
