#include "managerwidget.h"
#include "ui_managerwidget.h"


ManagerWidget::ManagerWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ManagerWidget)
{
    ui->setupUi(this);
    setWindowTitle("管理员界面");
    databaseWidget = new DatabaseWidget(this);
    ui->verticalLayout->addWidget(databaseWidget);
    connect(this,SIGNAL(tableView_model()),databaseWidget,SLOT(set_tableView_model()));
    connect(this,SIGNAL(export_table()),databaseWidget,SLOT(export_tableView()));
    connect(this,SIGNAL(print_table()),databaseWidget,SLOT(print_tabelView()));
}

ManagerWidget::~ManagerWidget()
{
    delete ui;
}

void ManagerWidget::on_actionaction_triggered(bool checked)
{
    Q_UNUSED(checked);
    table_name = "result";
    databaseWidget->model->setTable(table_name);
    emit this->tableView_model();
}

void ManagerWidget::on_actionaction_2_triggered(bool checked)
{
    Q_UNUSED(checked);
    table_name = "loginhistory";
    databaseWidget->model->setTable(table_name);
    emit this->tableView_model();
}

void ManagerWidget::on_actionaction_3_triggered(bool checked)
{
    Q_UNUSED(checked);
    table_name = "operator";
    databaseWidget->model->setTable(table_name);
    emit this->tableView_model();
}


void ManagerWidget::on_exportAction_triggered()
{
    emit this->export_table();
}

void ManagerWidget::on_printAction_triggered()
{
    emit this->print_table();
}
