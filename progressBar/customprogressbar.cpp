#include "CustomProgressBar.h"
#include <QPainter>
#include <QDebug>

CustomProgressBar::CustomProgressBar(QWidget *parent) : QWidget(parent),
    m_step_count(0),
    m_current_step(0),
    isStart(false)
{
    setMinimumWidth(300);
    this->stepsBrowser = new QTextBrowser(this);
    this->stepsBrowser->move(0,this->progress_height + 20);
    this->stepsBrowser->setFontPointSize(12);
}

void CustomProgressBar::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    this->stepsBrowser->setFixedSize(this->width(),this->height()-this->progress_height-20);

}

void CustomProgressBar::setStepCount(int count)
{
    m_step_count = count;
    update();
}

void CustomProgressBar::setStepTexts(const QStringList &texts)
{
    this->m_step_texts.clear();
    m_step_texts = texts;
    update();
}

void CustomProgressBar::setCurrentStep(int step, const QString &message)
{
    if(step > m_step_count) {
        return;
    }

    m_current_step = step;
    if(!message.isEmpty()) {
        qDebug() << message;
    }
    //显示提示信息
    if(!m_step_texts.isEmpty() && this->m_current_step < this->m_step_count && this->isStart)
    {
        QString text = m_step_texts[this->m_current_step];
//        QFontMetrics fm(painter.font());
//        int w = fm.horizontalAdvance(text);
//        if(w > text_width)
//        {
//            text_width = w;
//        }
//        painter.setPen(Qt::black);
//        painter.drawText(0,this->progress_height + 100,text);
        this->stepsBrowser->setText(text);

    }
    update();
}

void CustomProgressBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    if(this->isStart)
    {
        drawBackground(painter);
        drawProgress(painter);
        drawSteps(painter);
    }
    else
    {
        drawBackground(painter);
        drawSteps(painter);
    }


}

void CustomProgressBar::drawBackground(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rect(0, 0, width(), this->progress_height);
    painter.setPen(Qt::NoPen);

    QColor backgroundColor = qApp->palette().color(QPalette::Window);
    QBrush brush(backgroundColor);
//    QBrush brush(QColor("#E6E6E6"));
    painter.setBrush(brush);

    painter.drawRect(rect);
}

void CustomProgressBar::drawSteps(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing);

    if(m_step_count <= 0) {
        return;
    }
    int step_width = width() / m_step_count;
    for(int i=0; i<m_step_count; i++)
    {
        QBrush brush(QColor("#007AFF"));
        if(i < m_current_step && this->isStart)
        {
            brush.setColor(QColor("#FF5B5D"));
        } else
        {
            brush.setColor(QColor("#808080"));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(brush);
        painter.drawEllipse(QPoint(step_width*(i+1) - this->progress_height/2, this->progress_height/2),this->progress_height/2, this->progress_height/2);
    }
}

void CustomProgressBar::drawProgress(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing);
    int width_step = width()/m_step_count;
    if(m_step_count <= 0) {
        return;
    }
    QBrush brush(QColor("#54B681"));
    painter.setBrush(brush);
    for(int i = 0;i<=m_current_step && i<this->m_step_count;++i)
    {
         QRect rect(i*width_step,5,width_step - this->progress_height/2 +8 ,this->progress_height-10);
         painter.drawRect(rect);
    }
}

void CustomProgressBar::setStartFlag(bool flag)
{
    this->isStart = flag;
    update();
}
