#include "database.h"
#include <QSqlQuery>
#include <QMessageBox>

QSqlDatabase Database::db = QSqlDatabase::addDatabase ("QMYSQL");    // 创建静态数据库实例
QSqlQuery* Database::sql = NULL;

Database::Database()
{

}

bool Database::getConnection()
{
    // 数据库连接
    db.setHostName("127.0.0.1");                        // 绑定数据库IP路径
    db.setPort(3306);                                   // 绑定数据库端口号
    db.setDatabaseName("TDD");                        // 绑定ODBC数据源名称
    db.setUserName("root");                             // 绑定数据库系统用户名
    db.setPassword("123456");                       // 输入数据库系统登录密码
    if(!db.open ())
    {
        qDebug()<<"DataBase Error";
        qDebug()<<db.lastError().text();
        QMessageBox::warning(nullptr,"error","连接数据库失败！"+db.lastError().text());
        return false;
    }
    else
    {
        qDebug()<<" Database connects successed!";
        return true;
    }


}

void Database::quitConnection()
{
    db.close();
}
