#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QMessageBox>
#include "Camera/GetImageThread.h"
#include "Camera/MvCamera.h"
#include <QMutex>
#include "progressBar/customprogressbar.h"
#include <QTimer>
#include <QDateTime>
#include "MySQL/database.h"
#include <QSqlTableModel>
#include <QSplitter>
#include "visionAlgorithm/imagepro.h"
#include <QUdpSocket>
#include "tcpScoket/mytcpserver.h"
#include "displayLabel/mylabel.h"
#include "udpSocketThread/udpsocketthread.h"
#include <Qvector>
#include <QScrollBar>
#include <unordered_map>
#include <QTextCodec>
#include <QToolBar>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void  showEvent(QShowEvent *event);

    void resizeEvent(QResizeEvent *event);

    bool saveImage(cv::Mat& image);

    /**
     * @brief loadStepTipsDoc 读取文本中的全部项目的步骤提示 文本中空行为项目的标识 @为步的标识
     */
    void loadStepTipsDoc();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void myProgressBarInit(int index);                         //自定义进度条初始化

    void myProgressBarStepUpdate(int currentStep);          //更新自定义进度条

    void saveSettings();                              //关闭窗口是保存状态

    void loadSettings();                              //打开窗口是恢复状态

    void printCurrentDetectResult(int defaultIndex = -1);                  //打印每次检测的结果                 waiting for using

    void setLineEditReadOnly(bool isReadOnly = true); //设置结果显示item只读

    void clearResult();

    void on_GetParamButton_clicked();         //控件已删除

    //测试函数
    void test_gradLean();

    //图像处理函数
    bool detect_lines();

    bool detect_point(cv::Rect rect);

    void itemLayout(int index);              //布局 包括控件的布局以及自定义进度条的步骤提示信息的更新

    void setScrollBarWHOffset(int offsetX,int offsetY);

    QString degreeUnitTransfer(double value);

    void cameraParaInit(float exposeTime=800,float gain=4,float frameRate=10,bool autoExposeTime=false,bool autoGain=false);


public slots:
    void updateDisplayLabel(MV_FRAME_OUT newFrame);


private slots:
    void on_action_triggered();

    void onEnumButtonReleased();

    void onOpenButtonReleased();

    void onCloseButtonReleased();

    void onstartGetImageButtonReleased();

    void onstopGetImageButtonReleased();

    void onTimeout();

    void on_ExposureLineEdit_returnPressed();

    void on_GainLineEdit_returnPressed();

    void on_AcquisitionFrameRateLineEdit_returnPressed();

    void on_SaveBMPButton_clicked();

    void on_SaveJPGButton_clicked();

    void on_saveButton_clicked();

    void on_productIDLineEdit1_returnPressed();

    void on_tabWidget_tabBarClicked(int index);

    void on_checkBox_clicked(bool checked);

    void on_startGrabButton_clicked();

    void on_getImageButton_clicked();

    void on_imageProcessButton_clicked();

    void on_indicatorComputeButton_clicked();

    void on_detectItemComboBox_activated(int index);

    void matchLine(QPoint point);

    void matchPoint(cv::Rect rect);

    void splitterMove(int pos,int index);

    void on_checkBox_2_clicked(bool checked);

    void on_checkBox_3_clicked(bool checked);

    void on_lowLeftButton_pressed();

    void on_lowLeftButton_released();

    void on_lowRightButton_pressed();

    void on_lowRightButton_released();

    void on_resultLineEdit_4_returnPressed();

    void on_PDLineEdit_36_returnPressed();

    void on_rateLineEdit_5_returnPressed();

    void on_focusLineEdit_6_returnPressed();

    void on_P2LineEdit_7_returnPressed();

    void on_unitLineEdit_4_returnPressed();

    void on_minuteLineEdit_returnPressed();

    void on_lowLeftButton_2_pressed();

    void on_lowLeftButton_2_released();

    void on_lowRightButton_2_pressed();

    void on_lowRightButton_2_released();

    void on_focusConfirmButton_2_clicked();

    void on_ExposureLineEdit_3_returnPressed();

    void on_GainLineEdit_3_returnPressed();

    void on_AcquisitionFrameRateLineEdit_3_returnPressed();

    void on_leftLimitButton_clicked();

    void on_rightLimitButton_clicked();

    void on_productComboBox_activated(int index);

    void on_BarLeftButton_clicked();

    void on_BarRightButton_clicked();

    void on_action_2_triggered();

    void on_action_3_triggered();


    void on_zeroButton_clicked();

private:
    //数据库
    Database *myDatabase;
    //进度条
    CustomProgressBar *myProgressBar; //自定义进度条
    std::unordered_map<int,QStringList> stepTipsMap; //全部项目的提示信息
    QStringList m_steps;              //提示信息
    int step = 0;                     //完成步骤数
    int stepCount = 4;                //总步骤数
    //splitter
    QSplitter *splitter;
    QLabel *separator;
    QStringList degreeUnitList;      //角 分 秒单位

    //相机
    MvCamera *m_mvCamera;
    bool isAutoExposureTime = false;  //是否设定自动曝光
    bool isAutoGain = false;          //是否设定自动增益
    GetImageThread *m_getImageThread; // 图片抓取线程
    MV_CC_DEVICE_INFO_LIST *m_camDeviceList;
    cv::Mat image_GRAY;
    cv::Mat image_BGR;
    cv::Mat image_BGR_display;       //只用于显示的
    QString saveFormat;//图片保存格式 默认JPG
    QMutex m_mutex;

    //状态栏
    QTimer *myTimer ;
    QLabel *timeLabel;
    QToolBar *toolbar;

    //指标计算
    //指标结果保存变量
     QString productID ;
     QString gradLean;
     QString leftImageIncline;
     QString rightImageIncline;
     QString relImageIncline;
     QString hAxisParalDegree;
     QString vAxisParalDegree;
     QString leftParal;
     QString rightParal;
     QString leftAdjustRange;
     QString rightAdjustRange;
     QString leftVisZero;
     QString rightVisZero;
     QString leftResolution;
     QString rightResolution;
     QString leftField;
     QString rightField;
     QString leftDistortion;
     QString rightDistortion;
     QString leftExitPupil;
     QString rightExitPupil;
     QString leftExitPupilDistance;
     QString rightExitPupilDistance;
     QString leftRate;
     QString rightRate;
     QString rateDif;
     QString pupilDistanceRange;
     QString saveFilePath;
     QDateTime testTime;
     QString operatorID;

    //检测中过程变量
     double ndetectResult = 0;            //本次检测结果   分辨力秒
     double aveDetectResult = 0;          //本次检测均值或者视度零位(检测视差)
     double ndetectResult_vaxis = 0;       //垂直平行度检测结果  分辨力毫秒
     double aveDetectResult_vaxis = 0;    //垂直平行度均值或者视度零位(检测视差)
     int finishItemCount = 0;             //已检测完的项目数量
     int nCurrentCount = 0;               //当前检测次数
     int currentItemIndex = 0;            //当前项目的combox索引
     int currentProductType = 0;          //当前产品型号
     QVector<bool> finishItemFlags;       //标志已完成项目
     bool isDetectLeftLimit = false;      //是否检测视度调节范围左极限
     std::vector<double> leftImageInclineArray;//6次左像倾斜结果
     std::vector<double> rightImageInclineArray;//6次右像倾斜结果
     std::vector<double> leftRateArray;    //6次左倍率结果
     std::vector<double> rightRateArray;   //6次右倍率结果
     std::vector<double> leftVisZeroArray;    //6次左视度零位结果
     std::vector<double> rightVisZeroArray;   //6次右视度零位结果


     int testCount = 6;                   //检测总次数                    R
     int itemCount = 25;                  //项目总数为24                  R
     QStringList itemID;                  //项目编号汇总(保存图片设置图片名称使用)
     QList<QStringList> resolutionValue;  //A2 A3 A4 分辨力板

     //参数设置
     double pExitPupilDistance = 0;      //出瞳直径
     double pMagnification = 0;          //放大率
     double lenFocus = 0;                //镜头焦距
     double pP2 = 0;                     //P2
     double defocusDistanceZero = 0;     //光栅尺零点
     double defocusDistance = 0;         //离焦距离值
     double defaultMagnification = 1;    //默认的放大率，临时变量只是为了万一计算光轴平行度需要除以倍率准备

    //图像处理
     ImagePro *p;
     Ui::MainWindow *ui;
     std::vector<cv::Vec4i> detecteLines;  //hough 检测结果
     QPoint originCenterPos;               //模板匹配十字中心坐标

     bool isMatchRightPoint = false;       //是否已经匹配右侧十字
     QPoint leftOriginCenterPos;          //左镜筒匹配十字中心坐标

     //串口通信
     udpSocketThread *udpThread;

     //question 设置scrollBar位置标志
     bool isChecked = false;
     bool isClickToolBar = false;
     int singleStep = 5;                 //光栅尺单步一次点击走的步数



public:

};
#endif // MAINWINDOW_H
