/**************************************************
**
** Copyright (C) 2022 zhouxuan.
** Contact: 微信公众号【周旋机器视觉】
**
** framelesswindow文件 将窗体设为了无边框窗口，并重写了窗体的鼠标响应事件
** 使其具备正常窗口的鼠标响应动作（窗体拖拽边框调整大小、窗体鼠标拖动等）
** 并将窗体和标题栏分离，在构造函数里动态的添加标题栏，使得标题栏可以方便的进行
** 用户自定义，详情见titlebar文件
**
** 实现过程重点参考了以下两个优秀项目，在此声明并致谢：
** https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle
** https://www.cnblogs.com/xufeiyang/p/3313104.html
**
** LICENSE:LGPL
**************************************************/
#ifndef FRAMELESSWINDOW_H
#define FRAMELESSWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QMouseEvent>
#include <QPoint>
#include <QCursor>
#include <QRect>
#include "FramelessWindow/titlebar.h"
#include "mainwindow.h"

#define PADDING 11 //窗体边框的像素宽度

namespace Ui {
class FramelessWindow;
}

enum Direction{
    UP = 0,
    DOWN=1,
    LEFT,
    RIGHT,
    LEFTTOP,
    LEFTBOTTOM,
    RIGHTBOTTOM,
    RIGHTTOP,
    NONE
};

class FramelessWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FramelessWindow(QWidget *parent = nullptr);
    virtual ~FramelessWindow();
    void setContent(QWidget *w);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void region(const QPoint &cursorPoint);
    void styleWindow(bool bActive, bool bNoState);
protected:
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void changeEvent(QEvent *event);
    void timerEvent(QTimerEvent* e) ;

private:
    Ui::FramelessWindow *ui;
    TitleBar *titlebar;
    MainWindow *mainWindow;
    bool isLeftPressDown;
    QPoint dragPosition;
    Direction dir;
};

#endif // FRAMELESSWINDOW_H
