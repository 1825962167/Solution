/**************************************************
**
** titlebar自定义标题栏类，通过重写鼠标响应事件，实现以下功能：
** 1、拖动自定义标题栏可拖动窗口界面
** 2、双击标题栏窗口全屏放大， 再次双击窗口恢复正常大小
** 3、应用窗口全屏状态下，拖动标题栏可使窗口恢复正常大小并跟随拖动
** 标题栏界面布局可自定义，但应保留最基本的四个按钮实现最小化、最大化、关闭等
** 基本功能。
** 其它功能可通过添加控件和槽函数来自行设计填充
**
**************************************************/
#include "titlebar.h"
#include "ui_titlebar.h"
#include <QPainter>
#include <QStyleOption>
#include <QMenu>
#include <QDebug>
#include <QMessageBox>
#include "AppStyle/stylemanager.h"
#include "AppStyle/lightstyle.h"
#include <QPixmap>


TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::titlebar),
    parent_MainWindow(parent)
{
    ui->setupUi(this);
    mousePressed = false;
    last_width = 1000;
    ui->restoreButton->setVisible(false);
    ui->verticalLayout->setSpacing(0);
    InitDesigner();
}

TitleBar::~TitleBar()
{
    delete ui;
}

void TitleBar::InitDesigner() {
    //设置窗口图标

    QPixmap binoclularsImage = QPixmap(":/frameless_images/logo.png");
    binoclularsImage = binoclularsImage.scaled(ui->icon_label->size());
    ui->icon_label->setPixmap(binoclularsImage);

    //创建下拉菜单  换肤下拉菜单
    QMenu *skinButton_menu = new QMenu(this);
    skinButton_menu->addAction(ui->defaultstyle_action);
    skinButton_menu->addAction(ui->darkstyle_action);
    skinButton_menu->addAction(ui->lightstyle_action);
    skinButton_menu->addAction(ui->ubuntustyle_action);
    skinButton_menu->addAction(ui->macosstyle_action);

    ui->skinButton->setPopupMode(QToolButton::InstantPopup);
    ui->skinButton->setMenu(skinButton_menu);
}

void TitleBar::setTitleText(const QString &text) {
  ui->titleText_lable->setText(text);
}

//void TitleBar::setNoteText(const QString &text) {
//  ui->noteText_lable->setText(text);
//}

void TitleBar::setIcon(QString path) {
    ui->icon_label->setPixmap(QPixmap(path));
}

void TitleBar::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        last_width = parent_MainWindow->width();
    }
}


void TitleBar::mousePressEvent(QMouseEvent *event) {
  mousePressed = true;
  mousePos = event->globalPos();

  if (parent_MainWindow) wndPos = parent_MainWindow->pos();
}

void TitleBar::mouseMoveEvent(QMouseEvent *event) {
//  QWidget *parent = parentWidget();
//  if (parent) parent = parent->parentWidget();

  if (parent_MainWindow && mousePressed)
  {
      if(!parent_MainWindow->isMaximized())
      {
          //界面的全局位置 + （当前鼠标位置-鼠标按下时的位置）
          parent_MainWindow->move(wndPos + (event->globalPos() - mousePos));       
      }
      else
      {
          //窗口最大化时鼠标拖动标题栏使窗体复原，且鼠标相对窗体的相对位置不变
          //相对位置不变指的是：鼠标点击拖动窗口1/4处进行拖动，复原时鼠标依然位于窗口1/4处
          //达到此效果，仅需更改窗口的位置即可
          float width_ratio = float(event->globalPos().x())/float(parent_MainWindow->width());
          on_restoreButton_clicked();
          //更新窗体的位置，将鼠标点击的窗体标题栏的相对位置定位到鼠标当前位置
          int normal_width = last_width * width_ratio;
          int normal_higth = this->height()/4;
          wndPos = event->globalPos() - QPoint(normal_width,normal_higth);
      }
  }
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event) {
  Q_UNUSED(event);
  mousePressed = false;
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event);

    if (parent_MainWindow->windowState().testFlag(Qt::WindowNoState)) {
      on_maximizeButton_clicked();
    }
    else{
      on_restoreButton_clicked();
    }
}

void TitleBar::on_minimizeButton_clicked()
{
    ui->restoreButton->setVisible(false);
    ui->maximizeButton->setVisible(true);
    parent_MainWindow->setWindowState(Qt::WindowMinimized);
}

void TitleBar::on_restoreButton_clicked()
{
    ui->restoreButton->setVisible(false);
    ui->maximizeButton->setVisible(true);

    parent_MainWindow->setWindowState(Qt::WindowNoState);

}

void TitleBar::on_maximizeButton_clicked()
{
    ui->restoreButton->setVisible(true);
    ui->maximizeButton->setVisible(false);

    parent_MainWindow->setWindowState(Qt::WindowMaximized);//windowstate需要手动设置
    parent_MainWindow->showMaximized();
}

void TitleBar::on_closeButton_clicked()
{
    parent_MainWindow->close();
}

void TitleBar::on_defaultstyle_action_triggered()
{
    StyleManager::StyleUpdate(StyleTypes::NONE);

    ui->minimizeButton->setIcon(QIcon(QPixmap(":/frameless_images/icon_window_minimize.png")));
    ui->restoreButton->setIcon(QIcon(QPixmap(":/frameless_images/icon_window_restore.png")));
    ui->maximizeButton->setIcon(QIcon(QPixmap(":/frameless_images/icon_window_maximize.png")));
    ui->closeButton->setIcon(QIcon(QPixmap(":/frameless_images/icon_window_close.png")));
}
void TitleBar::on_darkstyle_action_triggered()
{
    // TODO: 换肤的设计还是比较差劲，代码不够简洁美观。由于样式文件直接覆盖整个应用程序
    //因此每次换肤需要单独对独立的titlebar的相关样式进行更新，这部分代码应该集成于titlebar文件自身
    // TODO: 可以通过为titlebar写一个单独的样式更新函数来提高代码的可阅读性
    StyleManager::StyleUpdate(StyleTypes::DARK);

    ui->minimizeButton->setIcon(QIcon(QPixmap(":/frameless_images/icon_window_minimize.png")));
    ui->restoreButton->setIcon(QIcon(QPixmap(":/frameless_images/icon_window_restore.png")));
    ui->maximizeButton->setIcon(QIcon(QPixmap(":/frameless_images/icon_window_maximize.png")));
    ui->closeButton->setIcon(QIcon(QPixmap(":/frameless_images/icon_window_close.png")));

}

void TitleBar::on_lightstyle_action_triggered()
{
    StyleManager::StyleUpdate(StyleTypes::LIGHT);
    ui->minimizeButton->setIcon(QIcon(QPixmap(":/lightstyle/light_window_minimize.png")));
    ui->restoreButton->setIcon(QIcon(QPixmap(":/lightstyle/light_window_restore.png")));
    ui->maximizeButton->setIcon(QIcon(QPixmap(":/lightstyle/light_window_maximize.png")));
    ui->closeButton->setIcon(QIcon(QPixmap(":/lightstyle/light_window_close.png")));
}

void TitleBar::on_ubuntustyle_action_triggered()
{
    LightStyle::setPalette();
    // loadstylesheet
    QFile qfDarkstyle(":/QSS-master/Ubuntu.qss");

    if (qfDarkstyle.open(QFile::ReadOnly)) {
      // set stylesheet
      qApp->setStyleSheet(qfDarkstyle.readAll());
      qfDarkstyle.close();
    }
}

void TitleBar::on_macosstyle_action_triggered()
{
    LightStyle::setPalette();
    // loadstylesheet
    QFile qfDarkstyle(":/QSS-master/MacOS.qss");

    if (qfDarkstyle.open(QFile::ReadOnly)) {
      // set stylesheet
      qApp->setStyleSheet(qfDarkstyle.readAll());
      qfDarkstyle.close();
    }
}

void TitleBar::on_noteButton_clicked()
{
    // TODO: 可以使用同样的方式为QMessageBox消息提示窗口设计一款无边框窗口，
    // TODO: 来达到与主窗口风格一致且可自定义的效果，此处仅使用QT原生QMessageBox
//    QMessageBox::information(this,"information",QString::fromUtf8("Contact: wechat and QQ."));
    QString fileName = "使用维护说明书.docx";
//    QString file_path = QDir("../SolutionRelease/doc").path()+"/"+fileName;
    QString file_path = QCoreApplication::applicationDirPath()+"/doc/"+fileName;
    QString abso_path = QDir::toNativeSeparators(QDir(file_path).absolutePath());
    QString urlPath = QUrl::fromLocalFile(abso_path).toString();
    if(urlPath.isEmpty())return;
    QDesktopServices::openUrl(urlPath);
}


