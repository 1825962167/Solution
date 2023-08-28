/**************************************************
**
** Copyright (C) 2022 zhouxuan.
** Contact: 微信公众号【周旋机器视觉】
**
** 亮色系皮肤文件
**
** LICENSE:LGPL
**************************************************/
#ifndef LIGHTSTYLE_H
#define LIGHTSTYLE_H

#include <QApplication>

class LightStyle{

public:
    LightStyle();
    static void polish();
    static void setPalette();

private:
    static void setQss();

};

#endif // LIGHTSTYLE_H
