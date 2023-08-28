/**************************************************
**
** Copyright (C) 2022 zhouxuan.
** Contact: 微信公众号【周旋机器视觉】
**
** 亮色系皮肤的实现，通过设置应用程序的 QPalette 调色板以及
** 控件的qss样式表，来实现界面的美化。
**
** LICENSE:LGPL
**************************************************/
#include "lightstyle.h"
#include <QFile>
#include <QStyle>
#include <QStyleFactory>

LightStyle::LightStyle()
{

}

void LightStyle::polish()
{
    setPalette();
    setQss();
}

void LightStyle::setPalette(){

    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setPalette(QApplication::style()->standardPalette());

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(242, 242, 242));
    palette.setColor(QPalette::WindowText, QColor(26, 26, 26));
    palette.setColor(QPalette::Disabled, QPalette::WindowText,
                     QColor(128, 128, 128));
    palette.setColor(QPalette::Base, QColor(230, 230, 230));
    palette.setColor(QPalette::AlternateBase, QColor(248, 248, 248));
    palette.setColor(QPalette::ToolTipBase, QColor(248, 248, 248));
    palette.setColor(QPalette::ToolTipText, QColor(51, 51, 51));
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    palette.setColor(QPalette::Dark, QColor(202, 202, 202));
    palette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    palette.setColor(QPalette::Button, QColor(230, 230, 230));
    palette.setColor(QPalette::ButtonText, Qt::black);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                     QColor(127, 127, 127));
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(243, 185, 23));
    palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                     QColor(127, 127, 127));

    qApp->setPalette(palette);
}

void LightStyle::setQss(){
    // loadstylesheet
    QFile qssfile(":/lightstyle/lightstyle.qss");

    if (qssfile.open(QFile::ReadOnly)) {
      // set stylesheet
      qApp->setStyleSheet(qssfile.readAll());
      qssfile.close();
    }
}
