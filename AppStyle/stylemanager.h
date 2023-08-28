/**************************************************
**
** Copyright (C) 2022 zhouxuan.
** Contact: 微信公众号【周旋机器视觉】
**
** StyleManager 样式管理，只需在需要的地方调用该文件的
** StyleUpdate()静态函数接口来设置当前应用程序的样式，
** 来达到换肤的功能
**
** LICENSE:LGPL
**************************************************/
#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

#include <QObject>
#include "AppStyle/DarkStyle.h"
#include "AppStyle/lightstyle.h"

enum class StyleTypes{
    NONE = 0,
    DARK = 1,
    LIGHT,
    UBUNTU,
    MACOS
};

class StyleManager: public QObject
{
    Q_OBJECT
public:
    StyleManager();
    static bool StyleUpdate(StyleTypes flag);


private:
    StyleTypes styletype;
};

#endif // STYLEMANAGER_H
