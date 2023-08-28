#ifndef MANAGERWIDGET_H
#define MANAGERWIDGET_H

#include <QMainWindow>
#include "databasewidget.h"
#include <QSqlTableModel>


namespace Ui {
class ManagerWidget;
}

class ManagerWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit ManagerWidget(QWidget *parent = nullptr);
    virtual ~ManagerWidget();

signals:
    void tableView_model();        //表名改变信号
    void export_table();           //导出信号
    void print_table();            //打印信号

private slots:
    void on_actionaction_triggered(bool checked);

    void on_actionaction_2_triggered(bool checked);

    void on_actionaction_3_triggered(bool checked);

    void on_exportAction_triggered();

    void on_printAction_triggered();

private:
    Ui::ManagerWidget *ui;
    DatabaseWidget *databaseWidget;
    QString table_name;

};

#endif // MANAGERWIDGET_H
