#include "registerwidget.h"
#include "ui_registerwidget.h"


RegisterWidget::RegisterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegisterWidget)
{
    ui->setupUi(this);
    this->MyDatabase.getConnection();
    setWindowFlags(Qt::Widget);
    setWindowTitle("注册");
    ui->operatorRadioButton->setChecked(true);
    ui->InputNumber->setFocus();
    //美化
    this->ui->label->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    this->ui->label_2->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    this->ui->label_3->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    this->ui->label_4->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    this->ui->operatorRadioButton->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    this->ui->managerRadioButton->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    ui->registerButton->setStyleSheet("QPushButton{font: 25 14pt '微软雅黑 Light';color: rgb(255,255,255);background-color: rgb(20,196,188);"
                                                                  "border: 2px groove gray;border-radius:15px;padding:2px 4px;border-style: outset;}"
                                                                  "QPushButton:hover{background-color: rgb(22,218,208);}"//hover
                                                                  "QPushButton:pressed{background-color: rgb(17,171,164);}"//pressed
                                                                    "border:2px solid rgb(20,196,188);");//边框粗细-颜色-圆角设置
    ui->backButton->setStyleSheet("QPushButton{font: 25 14pt '微软雅黑 Light';color: rgb(255,255,255);background-color: rgb(20,196,188);"
                                                                      "border: 2px groove gray;border-radius:15px;padding:2px 4px;border-style: outset;}"
                                                                      "QPushButton:hover{background-color: rgb(22,218,208);}"//hover
                                                                      "QPushButton:pressed{background-color: rgb(17,171,164);}"//pressed
                                                                        "border:2px solid rgb(20,196,188);");//边框粗细-颜色-圆角设置
    this->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(33, 191, 224, 255), stop:1 rgba(255, 255, 255, 255));");
}

RegisterWidget::~RegisterWidget()
{
    delete ui;
}

void RegisterWidget::on_registerButton_clicked()
{
    QString userInputtype;
    if(ui->operatorRadioButton->isChecked()){
        userInputtype = "operator";
    }
    else
    {
        userInputtype = "manager";
    }
    QString username = this->ui->InputNumber->text();
    QString password = this->ui->InputPassWord->text();
    QString surepassword = this->ui->InputPassWord_2->text();
    QSqlQuery query;
    //顾客注册
    if(ui->operatorRadioButton->isChecked()){
        query.exec("select 用户名 from operator where 用户名 = '"+username+"'");
        QString str1;
        while(query.next()){
            str1 = query.value("用户名").toString();
        }
        if(username.isEmpty()||password.isEmpty()||surepassword.isEmpty()){
            QMessageBox::information(NULL, "提示", "账号密码确认密码不得为空！", QMessageBox::Ok);
        }
        else if(!str1.isEmpty())
        {
            QMessageBox::information(NULL,"提示","该用户名已被注册，请换一个用户名！",QMessageBox::Ok);
            ui->InputNumber->clear();
            this->ui->InputPassWord->clear();
            this->ui->InputPassWord_2->clear();
            this->ui->InputNumber->setFocus();
        }
        else if(password!=surepassword)
        {
            QMessageBox::information(NULL, "提示", "密码和确认密码不同，请重新输入！", QMessageBox::Ok);
            this->ui->InputPassWord->clear();
            this->ui->InputPassWord_2->clear();
            this->ui->InputPassWord->setFocus();
        }
        else
        {
            query.prepare("insert into operator(用户名,密码)values(?,?)");

            query.addBindValue(username);
            query.addBindValue(password);
            bool test = query.exec();
            if(test)
            {
                QMessageBox::information(NULL, "提示", "注册成功！", QMessageBox::Ok);
                //this->close();
            }
            else
            {
                QMessageBox::information(NULL, "提示", "注册不成功，请重新注册！", QMessageBox::Ok);
            }
        }
    }
    //管理员注册
    else
    {
        query.exec("select 管理员名 from manager where 管理员名 = '"+username+"'");
        QString str1;
        while(query.next()){
            str1 = query.value("管理员名").toString();
        }
        if(username.isEmpty()||password.isEmpty()||surepassword.isEmpty()){
            QMessageBox::information(NULL, "提示", "账号密码确认密码不得为空！", QMessageBox::Ok);
        }
        else if(!str1.isEmpty())
        {
            QMessageBox::information(NULL,"提示","该管理员名已被注册，请换一个管理员名！",QMessageBox::Ok);
            ui->InputNumber->clear();
            this->ui->InputPassWord->clear();
            this->ui->InputPassWord_2->clear();
            this->ui->InputNumber->setFocus();
        }
        else if(password!=surepassword)
        {
            QMessageBox::information(NULL, "提示", "密码和确认密码不同，请重新输入！", QMessageBox::Ok);
            this->ui->InputPassWord->clear();
            this->ui->InputPassWord_2->clear();
            this->ui->InputPassWord->setFocus();
        }
        else
        {
            query.prepare("insert into manager(管理员名,密码)values(?,?)");

            query.addBindValue(username);
            query.addBindValue(password);
            bool test = query.exec();
            if(test)
            {
                QMessageBox::information(NULL, "提示", "注册成功！", QMessageBox::Ok);
                //this->close();
            }
            else
            {
                QMessageBox::information(NULL, "提示", "注册不成功，请重新注册！", QMessageBox::Ok);
            }
        }
    }
}

void RegisterWidget::on_backButton_clicked()
{
    this->~RegisterWidget();
    login *w = new login();
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}
