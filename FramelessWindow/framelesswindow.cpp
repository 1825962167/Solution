/**************************************************
**
** framelesswindow无边框窗口类，为整个应用程序的基本窗口类，具备
** 1、无边框扁平化窗口
** 2、可拖动边界调整窗口大小（基于窗体九宫格布局，自己改写鼠标响应事件）
** 3、具备窗体阴影效果
** 4、可自定义标题栏
**
**************************************************/
#include "framelesswindow.h"
#include "ui_framelesswindow.h"
#include <QGraphicsDropShadowEffect>

/*
 * todo :
 *  1.在titlebar中note中添加help文档
 *
 */

FramelessWindow::FramelessWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FramelessWindow),
    titlebar(new TitleBar(this)),
    mainWindow(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("望远镜数字自动检定系统");
    //【1】将窗口设为无边框窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    //【2】添加最小化按钮标志，获取正确的Windows本机处理最小化函数
    //若注释掉则无法响应点击系统任务栏软件最大最小化操作
    #if defined(Q_OS_WIN)
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
    #endif
    //【3】设置鼠标跟踪，以此来编写自己无边框自定义标题栏窗口的鼠标事件
    setMouseTracking(true);
    //【4】important to watch mouse move from all child widgets
    QApplication::instance()->installEventFilter(this);
    //【5】添加自定义的标题栏
    titlebar->setTitleText(QString::fromLocal8Bit("望远镜数字自动检定系统"));
//    titlebar->setNoteText(QString::fromLocal8Bit("光学设备指标检测"));
    ui->titlebarContent_widget->layout()->addWidget(titlebar);

    //【6】设置窗口的阴影效果
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsDropShadowEffect *windowShadow = new QGraphicsDropShadowEffect;
    windowShadow->setBlurRadius(9.0);
    windowShadow->setColor(palette().color(QPalette::Highlight));
    windowShadow->setOffset(0.0);
    ui->Content_widget->setGraphicsEffect(windowShadow);
    //【7】添加mainwidget
    mainWindow = new MainWindow();
    ui->windowContent_widget->layout()->addWidget(mainWindow);
    //参数初始化
    isLeftPressDown = false;
    this->dir = NONE;
    //全屏显示界面  涉及到边界问题，一定要等界面构造结束后
    this->titlebar->on_maximizeButton_clicked();
}

//定时触发
void FramelessWindow::timerEvent(QTimerEvent* e) {
    Q_UNUSED(e);
//    ui->windowContent_widget->layout()->addWidget(mainwindow);
    update();
//    std::cout<<"update"<<std::endl;
}


FramelessWindow::~FramelessWindow()
{
    delete ui;
}

void FramelessWindow::setContent(QWidget *w)
{
  ui->windowContent_widget->layout()->addWidget(w);
}

void FramelessWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        isLeftPressDown = false;
        if(dir != NONE) {
            this->releaseMouse();
            this->setCursor(QCursor(Qt::ArrowCursor));
            dir = NONE;
        }
    }
}

void FramelessWindow::mousePressEvent(QMouseEvent *event)
{
    switch(event->button()) {
    case Qt::LeftButton:
        isLeftPressDown = true;
        if(dir != NONE) {
            this->mouseGrabber();
        } else {
            dragPosition = event->globalPos() - this->frameGeometry().topLeft();
        }
        break;
//    case Qt::RightButton:
//        this->close();
//        break;
    default:
        QWidget::mousePressEvent(event);
    }
}

void FramelessWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint gloPoint = event->globalPos();
    QRect rect = this->rect();
    QPoint tl = mapToGlobal(rect.topLeft());
    QPoint rb = mapToGlobal(rect.bottomRight());

    if(!isLeftPressDown) {
        this->region(gloPoint);
    } else {

        if(dir != NONE) {
            QRect rMove(tl, rb);

            switch(dir) {
            case LEFT:
                if(rb.x() - gloPoint.x() <= this->minimumWidth())
                    rMove.setX(tl.x());
                else
                    rMove.setX(gloPoint.x());
                break;
            case RIGHT:
                rMove.setWidth(gloPoint.x() - tl.x());
                break;
            case UP:
                if(rb.y() - gloPoint.y() <= this->minimumHeight())
                    rMove.setY(tl.y());
                else
                    rMove.setY(gloPoint.y());
                break;
            case DOWN:
                rMove.setHeight(gloPoint.y() - tl.y());
                break;
            case LEFTTOP:
                if(rb.x() - gloPoint.x() <= this->minimumWidth())
                    rMove.setX(tl.x());
                else
                    rMove.setX(gloPoint.x());
                if(rb.y() - gloPoint.y() <= this->minimumHeight())
                    rMove.setY(tl.y());
                else
                    rMove.setY(gloPoint.y());
                break;
            case RIGHTTOP:
                rMove.setWidth(gloPoint.x() - tl.x());
                rMove.setY(gloPoint.y());
                break;
            case LEFTBOTTOM:
                rMove.setX(gloPoint.x());
                rMove.setHeight(gloPoint.y() - tl.y());
                break;
            case RIGHTBOTTOM:
                rMove.setWidth(gloPoint.x() - tl.x());
                rMove.setHeight(gloPoint.y() - tl.y());
                break;
            default:

                break;
            }
            this->setGeometry(rMove);
        }
        //此处会导致整个窗体都可以跟随鼠标移动
//        else {
//            move(event->globalPos() - dragPosition);
//            event->accept();
//        }
    }
    QWidget::mouseMoveEvent(event);
}

void FramelessWindow::region(const QPoint &cursorGlobalPoint)
{
    QRect rect = this->rect();
    QPoint tl = mapToGlobal(rect.topLeft());
    QPoint rb = mapToGlobal(rect.bottomRight());
    int x = cursorGlobalPoint.x();
    int y = cursorGlobalPoint.y();

    if(tl.x() + PADDING >= x && tl.x() <= x && tl.y() + PADDING >= y && tl.y() <= y) {
        // 左上角
        dir = LEFTTOP;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if(x >= rb.x() - PADDING && x <= rb.x() && y >= rb.y() - PADDING && y <= rb.y()) {
        // 右下角
        dir = RIGHTBOTTOM;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if(x <= tl.x() + PADDING && x >= tl.x() && y >= rb.y() - PADDING && y <= rb.y()) {
        //左下角
        dir = LEFTBOTTOM;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if(x <= rb.x() && x >= rb.x() - PADDING && y >= tl.y() && y <= tl.y() + PADDING) {
        // 右上角
        dir = RIGHTTOP;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if(x <= tl.x() + PADDING && x >= tl.x()) {
        // 左边
        dir = LEFT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    } else if( x <= rb.x() && x >= rb.x() - PADDING) {
        // 右边
        dir = RIGHT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    }else if(y >= tl.y() && y <= tl.y() + PADDING){
        // 上边
        dir = UP;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    } else if(y <= rb.y() && y >= rb.y() - PADDING) {
        // 下边
        dir = DOWN;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    }else {
        // 默认
        dir = NONE;
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}

//eventfilter 函数只是针对framelesswindow
bool FramelessWindow::eventFilter(QObject *obj, QEvent *event)
{
  if (isMaximized())
  {
    return QWidget::eventFilter(obj, event);
  }

  // check mouse move event when mouse is moved on any object
  if (event->type() == QEvent::MouseMove)
  {
    QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
    if (pMouse)
    {
      mouseMoveEvent(pMouse);
    }
  }
  // press is triggered only on frame window
  else if (event->type() == QEvent::MouseButtonPress && obj == this)
  {
    QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
    if (pMouse)
    {
      mousePressEvent(pMouse);
    }
  }
  else if (event->type() == QEvent::MouseButtonRelease)
  {
    if (isLeftPressDown)
    {
      QMouseEvent *pMouse = dynamic_cast<QMouseEvent *>(event);
      if (pMouse)
      {
        mouseReleaseEvent(pMouse);
      }
    }
  }
  return QWidget::eventFilter(obj, event);
}

void FramelessWindow::changeEvent(QEvent *event) {
  if (event->type() == QEvent::WindowStateChange) {
    if (windowState().testFlag(Qt::WindowMaximized)) {
      styleWindow(true, false);
      event->ignore();
    }
    else //if (windowState().testFlag(Qt::WindowNoState)) {
    {
      styleWindow(true, true);
      event->ignore();
    }
  }
  event->accept();
}

void FramelessWindow::styleWindow(bool bActive, bool bNoState) {
  if (bActive) {
    if (bNoState) {
      layout()->setContentsMargins(PADDING,PADDING,PADDING,PADDING);
      ui->Content_widget->setStyleSheet(QStringLiteral(
          "#Content_widget{border:1px solid palette(highlight); border-radius:5px "
          "5px 5px 5px; background-color:palette(Window);}"));
      QGraphicsEffect *oldShadow = ui->Content_widget->graphicsEffect();
      if (oldShadow) delete oldShadow;
      QGraphicsDropShadowEffect *windowShadow = new QGraphicsDropShadowEffect;
      windowShadow->setBlurRadius(9.0);
      windowShadow->setColor(palette().color(QPalette::Highlight));
      windowShadow->setOffset(0.0);
      ui->Content_widget->setGraphicsEffect(windowShadow);
    } else {
      layout()->setContentsMargins(0,0,0,0);
      ui->Content_widget->setStyleSheet(QStringLiteral(
          "#Content_widget{border:1px solid palette(dark); border-radius:0px 0px "
          "0px 0px; background-color:palette(Window);}"));
      QGraphicsEffect *oldShadow = ui->Content_widget->graphicsEffect();
      if (oldShadow) delete oldShadow;
      ui->Content_widget->setGraphicsEffect(nullptr);
    }
  } else {
    if (bNoState) {
      layout()->setContentsMargins(PADDING,PADDING,PADDING,PADDING);
      ui->Content_widget->setStyleSheet(QStringLiteral(
          "#Content_widget{border:1px solid #000000; border-radius:5px 5px 5px "
          "5px; background-color:palette(Window);}"));
      QGraphicsEffect *oldShadow = ui->Content_widget->graphicsEffect();
      if (oldShadow) delete oldShadow;
      QGraphicsDropShadowEffect *windowShadow = new QGraphicsDropShadowEffect;
      windowShadow->setBlurRadius(9.0);
      windowShadow->setColor(palette().color(QPalette::Shadow));
      windowShadow->setOffset(0.0);
      ui->Content_widget->setGraphicsEffect(windowShadow);
    } else {
      layout()->setContentsMargins(0,0,0,0);
      ui->Content_widget->setStyleSheet(QStringLiteral(
          "#Content_widget{border:1px solid palette(shadow); border-radius:0px "
          "0px 0px 0px; background-color:palette(Window);}"));
      QGraphicsEffect *oldShadow = ui->Content_widget->graphicsEffect();
      if (oldShadow) delete oldShadow;
      ui->Content_widget->setGraphicsEffect(nullptr);
    }
  }
}

void FramelessWindow::closeEvent(QCloseEvent *event)
{
    if(!this->mainWindow->close())
    {
        event->ignore();
        return;
    }
    event->accept();
}
