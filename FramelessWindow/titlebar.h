/**************************************************
**
** Copyright (C) 2022 zhouxuan.
** Contact: 微信公众号【周旋机器视觉】
**
** titlebar文件 负责自定义标题栏，标题栏的UI布局和样式可以完全自定义
** 除自定义的内容外，titlebar类还包含了窗口标题栏的常见功能，如双击标题栏
** 窗体全屏、拖动标题栏可拖动窗体、全屏状态下拖动标题栏可恢复正常尺寸等。
** titlebar还包含了设置标题栏的标志、标题等接口。
**
** LICENSE:LGPL
**************************************************/
#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QMouseEvent>
#include <QWidget>
#include <QDir>
#include <QDesktopServices>

namespace Ui {
class titlebar;
}

class TitleBar : public QWidget
{
    Q_OBJECT

//公共函数均可重写、自定义
public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();
    void InitDesigner();
    void setTitleText(const QString &text);
//    void setNoteText(const QString &text);
    void setIcon(QString path);

protected:
    virtual void changeEvent(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

protected:
 QPoint mousePos;
 QPoint wndPos;//界面全局位置
 bool mousePressed;
 int last_width;

//UI界面控件的信号槽函数，可自定义重新设计
private slots:
 void on_minimizeButton_clicked();

 void on_restoreButton_clicked();

 void on_closeButton_clicked();

 void on_darkstyle_action_triggered();

 void on_lightstyle_action_triggered();

 void on_ubuntustyle_action_triggered();

 void on_macosstyle_action_triggered();

 void on_noteButton_clicked();

 void on_defaultstyle_action_triggered();

public slots:
 void on_maximizeButton_clicked();

private:
    Ui::titlebar *ui;
    QWidget *parent_MainWindow;
};

#endif // TITLEBAR_H
