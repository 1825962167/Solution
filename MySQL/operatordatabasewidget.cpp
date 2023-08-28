#include "operatordatabasewidget.h"
#include "ui_operatordatabasewidget.h"

OperatorDatabaseWidget::OperatorDatabaseWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OperatorDatabaseWidget)
{
    ui->setupUi(this);
    showMaximized();//界面最大化显示
    setWindowTitle("数据库");
    databaseWidget = new DatabaseWidget(this);
    ui->verticalLayout->addWidget(databaseWidget);
    connect(this,SIGNAL(export_table()),databaseWidget,SLOT(export_tableView()));
    connect(this,SIGNAL(print_table()),databaseWidget,SLOT(print_tabelView()));
}

OperatorDatabaseWidget::~OperatorDatabaseWidget()
{
    delete ui;
}

void OperatorDatabaseWidget::on_actiondaochu_triggered()
{
    emit this->export_table();
}

void OperatorDatabaseWidget::on_actiondayin_triggered()
{
    emit this->print_table();
}
