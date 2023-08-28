/*
 * feat: 自定义进度条
 *
 */
#ifndef CUSTOMPROGRESSBAR_H
#define CUSTOMPROGRESSBAR_H

#include <QWidget>
#include <QTextBrowser>
#include <QApplication>

class CustomProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit CustomProgressBar(QWidget *parent = nullptr);

    void resizeEvent(QResizeEvent *event);

    /**
     * @brief setStepCount 设置进度条的步数
     * @param count 步数
     */
    void setStepCount(int count);

    /**
     * @brief setStepTexts 设置步骤提示文字
     * @param texts 步骤提示文字
     */
    void setStepTexts(const QStringList &texts);

    /**
     * @brief setCurrentStep 设置当前进度所在的步数
     * @param step 步数
     * @param message 操作提示信息
     */
    void setCurrentStep(int step, const QString &message = "");

    /**
      * @brief setStartFlag设置进度条的flag
      * @param flag 是否开始
      */
    void setStartFlag(bool flag);

private:
    /**
     * @brief paintEvent 绘制函数
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief drawBackground 绘制进度条背景
     * @param painter 画笔
     */
    void drawBackground(QPainter &painter);

    /**
     * @brief drawSteps 绘制步骤节点和提示文字
     * @param painter 画笔
     */
    void drawSteps(QPainter &painter);

    /**
     * @brief drawProgress 绘制进度条进度
     * @param painter 画笔
     */
    void drawProgress(QPainter &painter);



private:
    int m_step_count; // 步数
    int m_current_step; // 当前步数
    QStringList m_step_texts; // 步骤提示文字
    int progress_height = 20;  //进度条的高度即节点的直径
    bool isStart;
    QTextBrowser *stepsBrowser;
};

#endif // CUSTOMPROGRESSBAR_H
