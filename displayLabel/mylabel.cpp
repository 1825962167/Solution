#include "mylabel.h"

MyLabel::MyLabel(QWidget* parent) : QLabel(parent)
{

}

void MyLabel::mousePressEvent(QMouseEvent* event)
{
    start_pos_ = event->pos();
    // 获取缩放后的图片
    QPixmap scaledPixmap = pixmap()->scaled(size());
    // 计算原图和缩放后图片的比例关系
    wRatio = (float)pixmap()->width() / (float)scaledPixmap.width();
    hRatio = (float)pixmap()->height() / (float)scaledPixmap.height();
    // 将鼠标点击位置转换为原图中的位置
    originPosStart = QPoint((int)start_pos_.x() * wRatio, (int)start_pos_.y() * hRatio);
    qDebug()<<wRatio<<" "<<hRatio;
    if (event->button() == Qt::LeftButton && !this->isOnlyClick)
    {
        drawing_ = true;
        qDebug()<<"起始点: "<<start_pos_<<" origin: "<<originPosStart;
    }
    if(event->button() == Qt::LeftButton && this->isOnlyClick)
    {
        emit clickLine(originPosStart);
        qDebug()<<"[clicked] "<<start_pos_<<" origin: "<<originPosStart;
    }

}

void MyLabel::mouseMoveEvent(QMouseEvent* event)
{
    if (drawing_)
    {
        end_pos_ = event->pos();
        update();
    }
}

void MyLabel::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && drawing_)
    {
        drawing_ = false;
        originPosEnd = QPoint((int)event->pos().x() * wRatio, (int)event->pos().y() * hRatio);
        cv::Mat image;
        cv::cvtColor(QImageToMat(pixmap()->toImage()), image, cv::COLOR_RGBA2GRAY);
        //鼠标移动方向为斜下角
        cv::Rect rect;
        if(originPosEnd.x() >= image.cols || originPosEnd.y() >= image.rows)
        {
            QMessageBox::warning(nullptr,"提示","鼠标超出显示区域",QMessageBox::Yes);
            return;
        }
        if(originPosEnd.x() > originPosStart.x() && originPosEnd.y() > originPosStart.y())
        {
            rect = cv::Rect(originPosStart.x(),originPosStart.y(),originPosEnd.x()-originPosStart.x(),originPosEnd.y()-originPosStart.y());
            cv::Mat  roi = image(rect);
//            cv::imshow("roi",roi);
        }
        else
        {
//            QMessageBox::critical(nullptr,tr("error"),tr("鼠标应斜下角移动"));
            update();
            return ;
        }
        this->rectArea = rect;
        qDebug()<<"[rect] "<<rect.x<<" "<<rect.y<<" "<<rect.width<<" "<<rect.height;
        if(isNeedRect)
        {
            emit matchRect(rectArea);
        }
//        qDebug()<<"终止点: "<<event->pos()<<" origin: "<<originPosEnd;
        update();
    }
}

void MyLabel::paintEvent(QPaintEvent* event)
{
    QLabel::paintEvent(event);
    if (drawing_)
    {
        QPainter painter(this);
        painter.setPen(QPen(Qt::green, 2, Qt::SolidLine));
        painter.drawRect(QRect(start_pos_, end_pos_));
    }
}

cv::Mat MyLabel::QImageToMat(const QImage& image)
{
    return cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine()).clone();
}

QImage MyLabel::MatToQImage(const cv::Mat& image)
{
    return QImage(image.data, image.cols, image.rows, image.step, QImage::Format_Grayscale8);
}
