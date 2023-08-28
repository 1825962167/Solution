/*
 * @copyright  Copyright (c) 3304
 * @author:kaixin.wang && peiji.jin
 * @version: v1.0
 * @date: 2023.3.14
 */
#include "mainwindow.h"
#include <QApplication>
#include "FramelessWindow/framelesswindow.h"
#include "AppStyle/stylemanager.h"
#include "MySQL/login.h"
#include <QTextCodec>
#include <QApplication>
#include "MySQL/databasewidget.h"
#include "MySQL/operatordatabasewidget.h"
#include <QDir>
#include <iostream>
#include <QProcess>
#include <QNetworkProxyFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);   
    QNetworkProxyFactory::setUseSystemConfiguration(false);
    QApplication::addLibraryPath("./plugins");//添加mysql插件路径添加到应用程序
    //设置中文字体
//    a.setFont(QFont("Microsoft Yahei", 14));

    //设置中文编码
    //设置本地编码方式为utf-8
    #if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    #if _MSC_VER
        QTextCodec *codec = QTextCodec::codecForName("gbk");
    #else
        QTextCodec *codec = QTextCodec::codecForName("utf-8");
    #endif
        QTextCodec::setCodecForLocale(codec);
        QTextCodec::setCodecForCStrings(codec);
        QTextCodec::setCodecForTr(codec);
    #else
        QTextCodec *codec = QTextCodec::codecForName("utf-8");
        QTextCodec::setCodecForLocale(codec);
    #endif

    //启动MySQL服务
    QProcess process;
    process.start("net start mysql");
    if (process.waitForFinished())
    {
//        QByteArray output = process.readAllStandardOutput();
        QByteArray error = process.readAllStandardError();
        QTextCodec* codec1 = QTextCodec::codecForName("GBK"); // 指定编码格式
        if (error.isEmpty())
        {
            qDebug() << "MySQL server started.";
        }
        else
        {
            qDebug() << "错误信息:" << codec1->toUnicode(error.trimmed());
        }
    }
    else
    {
        QMessageBox::critical(nullptr,"error","启动MySQL服务超时或失败");
        return -1;
    }
    //通过样式管理器的接口来设置初始样式为亮色系皮肤
    StyleManager::StyleUpdate(StyleTypes::NONE);

    //创建无边框界面窗口并通过setContent()接口将主窗口嵌入
    FramelessWindow framelessWindow;
    framelessWindow.show();

    //数据库
//    login w;
//    w.show();

   //数据库界面
//     DatabaseWidget *w = new DatabaseWidget;
//     w->show();

   //操作员数据库界面
//     OperatorDatabaseWidget *w = new OperatorDatabaseWidget;
//     w->show();

   //管理员界面
//        ManagerWidget *w = new ManagerWidget;
//        w->show();

//        MainWindow *w =new MainWindow();
//        w->show();


    return a.exec();
}

