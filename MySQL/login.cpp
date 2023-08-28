#include "login.h"
#include "ui_login.h"

/*
 * todo:
 *  1.操作员界面
 *  2.管理员界面
 *  3.注册界面
 *
 */

login::login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    ui->OperatorRadioButton->setChecked(true);
    this->myDatabase.getConnection();
    setWindowTitle("登录");
    setWindowFlags(Qt::Widget);
    ui->InputNumber->setFocus();
    //美化
    ui->label->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    ui->label_2->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    ui->label_3->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    this->ui->OperatorRadioButton->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    this->ui->ManagerRadioButton->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
    this->ui->InputNumber->setStyleSheet("font: 25 10pt '微软雅黑 Light';" //字体
                                         "color: rgb(31,31,31);"		//字体颜色
                                         "padding-left:20px;"       //内边距-字体缩进
                                         "background-color: rgb(255, 255, 255);" //背景颜色
                                         "border:2px solid rgb(20,196,188);border-radius:15px;");//边框粗细-颜色-圆角设置
    this->ui->InputPassWord->setStyleSheet("font: 25 10pt '微软雅黑 Light';" //字体
                                         "color: rgb(31,31,31);"		//字体颜色
                                         "padding-left:20px;"       //内边距-字体缩进
                                         "background-color: rgb(255, 255, 255);" //背景颜色
                                         "border:2px solid rgb(20,196,188);border-radius:15px;");//边框粗细-颜色-圆角设置
    this->ui->InputPassWord->setEchoMode(QLineEdit::Password);
    ui->LoginButton->setStyleSheet("QPushButton{font: 25 14pt '微软雅黑 Light';color: rgb(255,255,255);background-color: rgb(20,196,188);"
                                                                  "border: 2px groove gray;border-radius:15px;padding:2px 4px;border-style: outset;}"
                                                                  "QPushButton:hover{background-color: rgb(22,218,208);}"//hover
                                                                  "QPushButton:pressed{background-color: rgb(17,171,164);}"//pressed
                                                                    "border:2px solid rgb(20,196,188);");//边框粗细-颜色-圆角设置
    ui->RegisterButton->setStyleSheet("QPushButton{font: 25 14pt '微软雅黑 Light';color: rgb(255,255,255);background-color: rgb(20,196,188);"
                                                                      "border: 2px groove gray;border-radius:15px;padding:2px 4px;border-style: outset;}"
                                                                      "QPushButton:hover{background-color: rgb(22,218,208);}"//hover
                                                                      "QPushButton:pressed{background-color: rgb(17,171,164);}"//pressed
                                                                        "border:2px solid rgb(20,196,188);");//边框粗细-颜色-圆角设置
    ui->CloseButton->setStyleSheet("QPushButton{font: 25 14pt '微软雅黑 Light';color: rgb(255,255,255);background-color: rgb(20,196,188);"
                                                                      "border: 2px groove gray;border-radius:15px;padding:2px 4px;border-style: outset;}"
                                                                      "QPushButton:hover{background-color: rgb(22,218,208);}"//hover
                                                                      "QPushButton:pressed{background-color: rgb(17,171,164);}"//pressed
                                                                        "border:2px solid rgb(20,196,188);");//边框粗细-颜色-圆角设置
    //设置背景颜色
    this->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(33, 191, 224, 255), stop:1 rgba(255, 255, 255, 255));");

}

login::~login()
{
    delete ui;
}

void login::on_LoginButton_clicked()
{
    QString userInputtype;//判断是操作员还是管理员
    if(ui->OperatorRadioButton->isChecked()){// isChecked()来查询按钮是否被选中。
        userInputtype = "operator";
    }
    else
    {
        userInputtype = "manager";
    }
    QString username = this->ui->InputNumber->text();
    QString password = this->ui->InputPassWord->text();
    QSqlQuery query;
    if(ui->OperatorRadioButton->isChecked()){
        if(username.isEmpty()||password.isEmpty()){
            QMessageBox::information(NULL, "提示", "账号密码不得为空！", QMessageBox::Ok);
        }
        else
        {
            query.exec("select * from operator where 用户名 = '"+username+"'");
            if(!query.isActive())
            {
                QMessageBox::critical(NULL,"错误","查询未执行！", QMessageBox::Ok);
            }
            QString str1,str2;
            while(query.next()){
                str1 = query.value("用户名").toString();
                str2 = query.value("密码").toString();
            }
            if(str1.isEmpty()){
                QMessageBox::information(NULL, "提示", "没有该账号！", QMessageBox::Ok);
                this->ui->InputNumber->clear();
                this->ui->InputPassWord->clear();
                this->ui->InputNumber->setFocus();//输入焦点转为输入账号
            }
            else if(str2!=password)
            {
                QMessageBox::information(NULL, "提示", "账号存在，密码错误！", QMessageBox::Ok);
                this->ui->InputPassWord->clear();
                this->ui->InputPassWord->setFocus();
            }
            else
            {
                  this->~login();//调用析构函数，释放

                //登录成功 切换操作员页面
                //创建无边框界面窗口并通过setContent()接口将主窗口嵌入
                FramelessWindow *framelessWindow = new FramelessWindow();
                framelessWindow->setAttribute(Qt::WA_DeleteOnClose);
                framelessWindow->show();
                //插入登录记录
                QDateTime dateTime = QDateTime::currentDateTime();
                QString current_time = dateTime.toString("yyyy-MM-dd hh:mm:ss");
                query.prepare("insert into loginhistory(用户名,密码,登陆时间)values(?,?,?)");
                query.addBindValue(username);
                query.addBindValue(password);
                query.addBindValue(current_time);
                bool isinsert = query.exec();
                if(!isinsert)
                {
                    qDebug()<<"Failed to insert the login log."<<endl;
                }
            }
        }
    }
    else
    {
        if(username.isEmpty()||password.isEmpty()){
            QMessageBox::information(NULL, "提示", "账号密码不得为空！", QMessageBox::Ok);
        }
        else
        {
            query.exec("select * from manager where 管理员名 = '"+username+"'");
            if(!query.isActive())
            {
                QMessageBox::critical(NULL,"错误","查询未执行！", QMessageBox::Ok);
            }
            QString str1,str2;
            while(query.next()){
                str1 = query.value("管理员名").toString();
                str2 = query.value("密码").toString();
            }
            if(str1.isEmpty()){
                QMessageBox::information(NULL, "提示", "没有该账号！", QMessageBox::Ok);
                this->ui->InputNumber->clear();
                this->ui->InputPassWord->clear();
                this->ui->InputNumber->setFocus();//输入焦点转为输入账号
            }
            else if(str2!=password)
            {
                QMessageBox::information(NULL, "提示", "账号存在，密码错误！", QMessageBox::Ok);
                this->ui->InputPassWord->clear();
                this->ui->InputPassWord->setFocus();
            }
            else
            {
                this->~login();//调用析构函数，释放

                //登录成功切换管理员界面
                ManagerWidget *w = new ManagerWidget();
                w->setAttribute(Qt::WA_DeleteOnClose);
                w->show();
            }
        }
    }
}

void login::on_RegisterButton_clicked()
{
    this->~login();
    //切换至注册界面
    RegisterWidget *w = new RegisterWidget();
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();

}

void login::on_CloseButton_clicked()
{
    exit(0);
}
