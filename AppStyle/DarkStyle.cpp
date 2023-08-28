/**************************************************
**
** Copyright (C) 2022 zhouxuan.
** Contact: 微信公众号【周旋机器视觉】
**
** 暗色系皮肤的实现，通过设置应用程序的 QPalette 调色板以及
** 控件的qss样式表，来实现界面的美化。
**
** LICENSE:LGPL
**************************************************/
#include "DarkStyle.h"
#include <QFile>
#include <QStyle>
#include <QStyleFactory>

DarkStyle::DarkStyle()
{

}

void DarkStyle::polish()
{
    setPalette();
    setQss();
}

void DarkStyle::setPalette(){
    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setPalette(QApplication::style()->standardPalette());

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::WindowText,
                   QColor(127, 127, 127));
    palette.setColor(QPalette::Base, QColor(42, 42, 42));
    palette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, QColor(53, 53, 53));
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    palette.setColor(QPalette::Dark, QColor(35, 35, 35));
    palette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::ButtonText, Qt::white);
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

void DarkStyle::setQss(){

  // 加载样式表
  QFile qssfile(":/darkstyle/darkstyle.qss");

  if (qssfile.open(QFile::ReadOnly)) {
    // set stylesheet
    qApp->setStyleSheet(qssfile.readAll());
    qssfile.close();
  }
}
