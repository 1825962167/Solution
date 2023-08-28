#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <opencv2/opencv.hpp>
#include <QDebug>
#include <QMessageBox>

class MyLabel : public QLabel
{

    Q_OBJECT

public:
    MyLabel(QWidget* parent = nullptr);

signals:
    void clickLine(QPoint point);

    void matchRect(cv::Rect rect);

protected:
    void mousePressEvent(QMouseEvent* event) override ;

    void mouseMoveEvent(QMouseEvent* event) override ;

    void mouseReleaseEvent(QMouseEvent* event) override ;

    void paintEvent(QPaintEvent* event) override ;

public:
    cv::Mat QImageToMat(const QImage& image);

    QImage MatToQImage(const cv::Mat& image) ;



private:
    QPoint start_pos_;
    QPoint end_pos_;
    bool drawing_ = false;
    float wRatio,hRatio;                  //放缩比例
    QPoint originPosStart,originPosEnd;   //原始起始点、原始终止点

public:
    bool isOnlyClick = false;             //是否只能点击选择直线，区分
    bool isNeedRect = false;               //是否需要框选区域
    cv::Rect rectArea = cv::Rect();       //鼠标选择区域

};

#endif // MYLABEL_H
