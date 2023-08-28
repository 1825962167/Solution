/**************************************************
**
** Copyright (C) 2022 zhouxuan.
** Contact: 微信公众号【周旋机器视觉】
**
** stylemanager文件，样式管理器类，通过StyleUpdate()接口根据
** 传入参数修改应用程序的样式，且方便扩展，当需要添加新的样式时，只需要
** 参照darkstyle文件实现该样式的样式类，并在stylemanager样式管理器类
** 中添加对应的枚举类型并更新StyleUpdate()接口即可。
**
** LICENSE:LGPL
**************************************************/
#include "stylemanager.h"
#include "QStyleFactory"
#include "QApplication"
#include "QStyle"

StyleManager::StyleManager()
{

}

bool StyleManager::StyleUpdate(StyleTypes flag)
{
    switch (flag) {
    case StyleTypes::NONE:
        qApp->setStyle(QStyleFactory::create("Fusion"));
        qApp->setPalette(QApplication::style()->standardPalette());
        qApp->setStyleSheet(Q_NULLPTR);
        qApp->setStyleSheet(QString());
        break;
    case StyleTypes::DARK:
        DarkStyle::polish();
        break;
    case StyleTypes::LIGHT:
        LightStyle::polish();
        break;
    case StyleTypes::MACOS:

        break;
    case StyleTypes::UBUNTU:

        break;
    }
    return true;
}
