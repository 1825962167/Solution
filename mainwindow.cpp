#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "MySQL/operatordatabasewidget.h"
#include <QDir>
#include <vector>
#include <QSettings>


/*
 * todo :
 *  1.数据存档   got
 *      图片保存时，图片存储在assets文件下，数据库保存图片保存路径
 */


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      myDatabase(nullptr),
      myProgressBar(nullptr),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //初始化图像处理
    this->p = new ImagePro;
    //数据库初始化
    this->myDatabase = new Database();
    this->myDatabase->getConnection();

    // 初始化 相机类 图片抓取线程
    m_mvCamera = new MvCamera();
    m_getImageThread = new GetImageThread(&m_mvCamera);
    this->saveFormat = "JPG";//保存格式
    qRegisterMetaType<MV_FRAME_OUT>("MV_FRAME_OUT");//信号槽里面没有MV_FRAME_OUT类型，需要注册

    // 查找设备的按钮
    connect(ui->enumButton, &QPushButton::released, this, &MainWindow::onEnumButtonReleased);
    connect(ui->openButton, &QPushButton::released, this, &MainWindow::onOpenButtonReleased);
    connect(ui->closeButton, &QPushButton::released, this, &MainWindow::onCloseButtonReleased);
    connect(ui->startGetImageButton, &QPushButton::released, this, &MainWindow::onstartGetImageButtonReleased);
    connect(ui->stopGetImageButton, &QPushButton::released, this, &MainWindow::onstopGetImageButtonReleased);
    //自动增益和自动曝光槽函数复用
    connect(ui->checkBox_4,SIGNAL(clicked(bool)),this,SLOT(on_checkBox_2_clicked(bool)));
    connect(ui->checkBox_5,SIGNAL(clicked(bool)),this,SLOT(on_checkBox_3_clicked(bool)));
    // 连接抓取图片线程的信号，和，主线程的槽
    connect(m_getImageThread, &GetImageThread::newFrameOut, this, &MainWindow::updateDisplayLabel);
    //设置label居中显示图片
    ui->displayLabel->setAlignment(Qt::AlignCenter);
    //设置图片填充label
//    ui->displayLabel->setScaledContents(true);

    //进度条配置
    //提升widget为自定义的进度条类
    this->myProgressBar = new CustomProgressBar();
    //进度条初始化
    loadStepTipsDoc();
    myProgressBarInit(this->currentItemIndex);

    //splitter控件配置
    this->splitter = new QSplitter(Qt::Horizontal, this);
    ui->horizontalLayout_9->addWidget(splitter);
    splitter->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    splitter->setHandleWidth(0);//设置拆分器控件的分隔符宽度为0
    splitter->addWidget(ui->groupBox_13);
    this->separator = new QLabel(this);
    separator->setStyleSheet("background-color: black;");
    separator->setFixedWidth(0);
    splitter->addWidget(separator);
    splitter->addWidget(ui->displayGroup);
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(2,1);
    connect(splitter,SIGNAL(splitterMoved(int,int)),this,SLOT(splitterMove(int,int)));

    //状态栏初始化
    statusBar()->showMessage(tr("相机调试..."));
    //状态栏添加action
    this->toolbar = new QToolBar(this);
    this->toolbar->setStyleSheet("QToolBar { border: 1px solid rgb(200, 200, 200); }");
    this->toolbar->addAction(ui->action_2);
    this->toolbar->addAction(ui->action_3);
    this->toolbar->addAction(ui->action);
    statusBar()->addPermanentWidget(this->toolbar,0);
    timeLabel = new QLabel(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"), this);
    statusBar()->addPermanentWidget(timeLabel);
    //定时器
    myTimer = new QTimer(this);
    myTimer->start(1000);
    //每隔一秒更新时间
    connect(myTimer,SIGNAL(timeout()),this,SLOT(onTimeout()));

    //设置结果显示控件只读
    setLineEditReadOnly();
    //加载itemID
    this->itemID<<"A1.1"<<"A1.2"<<"A2.1"<<"A1.2"<<"A3.1"<<"A3.2"<<"A4"<<"A5.1"<<"A5.2"
               <<"A6"<<"A7"<<"A8.1"<<"A8.2"<<"A9.1"<<"A9.2"<<"A10"
              <<"A11.1"<<"A11.2"<<"A12.1"<<"A12.2"<<"A13"<<"A14.1"<<"A4.2"<<"A15.1"<<"A15.2";

    //登录加载操作员ID
    QSqlQuery query;
    query.exec("select 用户名 from loginhistory order by 用户名 desc limit 1");
    while(query.next())
    {
        this->operatorID = query.value(0).toString();
        ui->operatorIDLineEdit->setText(this->operatorID);
    }

    //指标计算
    connect(ui->displayLabel,SIGNAL(clickLine(QPoint)),this,SLOT(matchLine(QPoint)));
    connect(ui->displayLabel,SIGNAL(matchRect(cv::Rect)),this,SLOT(matchPoint(cv::Rect)));

    //UI布局
    ui->cameraSetup->layout()->setAlignment(Qt::AlignTop);
    ui->testItems->layout()->setAlignment(Qt::AlignTop);
    ui->horizontalLayout_PD->setAlignment(Qt::AlignRight);
    ui->horizontalLayout_rate->setAlignment(Qt::AlignRight);
    ui->horizontalLayout_focus->setAlignment(Qt::AlignRight);
    ui->horizontalLayout_P2->setAlignment(Qt::AlignRight);
    //设置ui结果输入的单位label
    ui->degreeLabel->setText("\u00B0");
    ui->minuteLabel->setText("\u2032");

    //角度单位list  度 分 秒
    this->degreeUnitList<<"\u00B0"<<"\u2032"<<"\u2033";
    //分辨力板分辨力
    QStringList tempA2,tempA3,tempA4;
    tempA2<<"33\u203306"<<"31\u203320"<<"29\u203347"<<"27\u203381"<<"26\u203324"<<"24\u203376"<<"23\u203339"<<"22\u203307"<<"20\u203383"<<"19\u203367"
       <<"18\u203356"<<"17\u203352"<<"16\u203353"<<"15\u203362"<<"14\u203371"<<"13\u203389"<<"13\u203310"<<"12\u203352"<<"11\u203370"<<"11\u203303"
         <<"10\u203341"<<"9\u203384"<<"9\u203326"<<"8\u203376"<<"8\u203327";
    tempA3<<"16\u203353"<<"15\u203362"<<"14\u203371"<<"13\u203389"<<"13\u203310"<<"12\u203340"<<"11\u203370"<<"11\u203303"<<"10\u203341"<<"9\u203384"
            <<"9\u203326"<<"8\u203376"<<"8\u203327"<<"7\u203381"<<"7\u203336"<<"6\u203394"<<"6\u203357"<<"6\u203320"<<"5\u203383"<<"5\u203350"
              <<"5\u203321"<<"4\u203392"<<"4\u203363"<<"4\u203338"<<"4\u203313";
    tempA4<<"8\u203327"<<"7\u203381"<<"7\u203336"<<"6\u203394"<<"6\u203357"<<"6\u203320"<<"5\u203383"<<"5\u203350"<<"5\u203321"<<"4\u203392"
            <<"4\u203363"<<"4\u203338"<<"4\u203313"<<"3\u203390"<<"3\u203368"<<"3\u203348"<<"3\u203328"<<"3\u203310"<<"2\u203392"<<"2\u203376"
              <<"2\u203360"<<"2\u203346"<<"2\u203332"<<"2\u203319"<<"2\u203307";
    this->resolutionValue<<tempA2<<tempA3<<tempA4;

    //左右像倾斜结果过程变量
    this->leftImageInclineArray = std::vector<double>(6,0);
    this->rightImageInclineArray = std::vector<double>(6,0);
    this->leftRateArray = std::vector<double>(6,0);
    this->rightRateArray = std::vector<double>(6,0);
    this->leftVisZeroArray = std::vector<double>(6,0);
    this->rightVisZeroArray = std::vector<double>(6,0);

    //创建finishItemFlags
    finishItemFlags = QVector<bool>(this->itemCount,false);

    //恢复前状态
    loadSettings();


    //udp串口
    this->udpThread = new udpSocketThread(this);
    if(this->udpThread->bindLocalPort())
    {
        this->udpThread->start();
    }
    else
    {
        ui->plainTextEdit->appendHtml("<font size=\"4\" color=\"red\">未连接上本地端口10006，请检查端口是否被占用</font>");
    }



    //测试保存测试图片
//    cv::Mat image;
//    QFile file("D:\\project_2022\\code\\12.JPG");
//    if(file.open(QFile::ReadOnly))
//    {
//        QByteArray bit = file.readAll();
//        image = cv::imdecode(std::vector<char>(bit.begin(),bit.end()),1);
//        if(saveImage(image))
//        {
//            qDebug()<<"保存图片成功"<<endl;
//        }
//    }
//    else
//        qDebug()<<"读取失败";

    QImage testImagel = QImage("D:\\project_2022\\code\\SolutionRelease\\FramelessWindow\\frameless_images\\Image_20230321185959564.bmp");
    ui->displayLabel->setScaledContents(true);
    ui->displayLabel->setPixmap(QPixmap::fromImage(testImagel));

    //测试分划倾斜
//    test_gradLean();

    qDebug()<<finishItemFlags;
    qDebug()<<"finishItemCount:"<<this->finishItemCount;


}

void MainWindow::test_gradLean()
{
    cv::Mat image;
    QFile file("D:\\project_2022\\code\\SolutionRelease\\FramelessWindow\\frameless_images\\Image_20230321185959564.bmp");
    if(file.open(QFile::ReadOnly))
    {
        QByteArray bit = file.readAll();
        image = cv::imdecode(std::vector<char>(bit.begin(),bit.end()),1);
    }
    else
        qDebug()<<"读取失败";
    cv::Mat image_gray;
    cv::cvtColor(image,image_gray,cv::COLOR_BGR2GRAY);
    //测试分划倾斜计算
    p->gaussianBlur(image_gray);
    p->enhanceContrast(image_gray);
    p->binarization(image_gray,50);
    p->canny(image_gray);
    p->dilateErode(image_gray);
    std::vector<cv::Vec4i> lines = p->hough(image_gray,50);
    p->paintHoughResult(image,lines);
    ui->displayLabel->setPixmap(QPixmap::fromImage(p->cvMat2QImage(image)));
    cv::namedWindow("temp",cv::WINDOW_FREERATIO);
    cv::imshow("temp",image_gray);

}

void MainWindow::myProgressBarInit(int index)
{
    this->m_steps.clear();
//    m_steps << "Step1 :\n"
//               "1、开机后，进入【相机设置】，依次单击【查找设备】、【打开设备】;\n"
//               "2、零位确定(未放上产品)\n"
//               "    a)单击【开始采集】;\n"
//               "    b)默认设置相机参数，设置【曝光】为800、【增益】为4、【帧率】为10;\n"
//               "    c)默认关闭【自动曝光】、【自动增益】;\n"
//               "    d)左右控制【光栅尺】直至平行光管的十字分划线最清晰;\n"
//               "    e)单击【零位确定】，计算机将自动保存零位位置,后续所有的检测都以该零位为基准;\n"
//               "3、进入【测试指标】\n"
//               "4、填写【产品信息】\n"
//               "    a)下拉选择产品类型,计算机会自动导入产品的参数信息;\n"
//               "    b)输入产品编号，输入以回车键结束;\n"
//               "    c)下拉选择检测项目;\n";
//    //左、右视差
//    if(index == 0 || index == 1)
//    {
//        m_steps << "Step2:\n"
//                   "1、确认【参数设置】中的参数是否正确，如需修改直接输入，按回车键结束;\n"
//                   "    a)7x50产品参数默认初始化为:【瞳距直径】为50mm、【放大率】为7X、【镜头焦距】为50mm、【P2】为32mm;\n"
//                   "    b)7x40产品参数默认初始化为:【瞳距直径】为40mm、【放大率】为7X、【镜头焦距】为50mm、【P2】为32mm;\n"
//                   "    c)6x30产品参数默认初始化为:【瞳距直径】为30mm、【放大率】为6X、【镜头焦距】为50mm、【P2】为32mm;\n"
//                << "Step3:\n"
//                   "1、打开【自动曝光】、【自动增益】;\n"
//                   "2、将产品的视度调至0屈光度;\n"
//                   "3、单击【开始采集】;\n"
//                   "4、将平行光管的十字分划移动至图像中中间区域，左右控制【光栅尺】来调整相机位置,直至平行光管的十字分划线最清晰;\n"
//                   "5、单击【停止采集】\n"
//                << "Step4:\n"
//                   "1、单击【指标计算】，自动计算出视差，检测结果显示在【应用程序输出】中\n"
//                   "重复上述试验6次，考核视差检测的重复性;";
//    }
//    //左右视度调节范围检测
//    if(index == 2 || index == 3)
//    {
//        m_steps << "Step2:\n"
//                   "1、打开【自动曝光】、【自动增益】;\n"
//                   "2、点击【开始采集】;\n"
//                   "3、将检验合格的陪试品（望远镜）视度调至-5屈光度;\n"
//                   "4、左右控制【光栅尺】来调整相机位置,直至平行光管的十字分划线最清晰;\n"
//                   "5、单击【停止采集】;\n"
//                   "6、单击【计算左极限】,计算机将自动测算适度调节范围的左极限;\n"
//                << "Step3:\n"
//                   "1、单击【开始采集】;\n"
//                   "2、在摄像机前安装一块-4屈光度的附加透镜，将检验合格的陪试品（望远镜）视度调至+1屈光度;\n"
//                   "3、将平行光管的十字分划移动至图像中中间区域，左右控制【光栅尺】来调整相机位置,直至平行光管的十字分划线最清晰;\n"
//                   "4、单击【停止采集】;\n"
//                   "5、单击【计算右极限】,计算机将自动测算适度调节范围的右极限,检测结果显示在【应用程序输出】中;\n"
//                   "重复上述试验6次，考核视度调节范围检测的重复性;";
//    }
//    //左右视度零位偏差检测
//    if(index == 4 || index == 5)
//    {
//        m_steps << "Step2:\n"
//                   "1、确认【参数设置】中的参数是否正确，参数默认初始化为:【镜头焦距】为50mm、【P2】为32mm,如需修改直接输入，按回车键结束;\n"
//                << "Step3:\n"
//                   "1、打开【自动曝光】、【自动增益】;\n"
//                   "2、将产品的视度调至0屈光度;\n"
//                   "3、单击【开始采集】;\n"
//                   "4、将平行光管的十字分划移动至图像中中间区域，左右控制【光栅尺】来调整相机位置,直至平行光管的十字分划线最清晰;\n"
//                   "5、单击【停止采集】\n"
//                << "Step4:\n"
//                   "1、单击【指标计算】，自动计算视度零位偏差，检测结果显示在【应用程序输出】中\n"
//                   "重复上述试验6次，考核视度零位偏差检测的重复性;";
//    }
//    //分划倾斜检测
//    if(index == 6)
//    {
//        m_steps << "Step2:\n"
//                   "1、打开【自动曝光】、【自动增益】;\n"
//                   "2、单击【开始采集】;\n"
//                   "3、用铅垂线作为基准，调节摄像机电子分划竖线与铅垂线一致;\n"
//                   "4、水平放置产品;\n"
//                   "5、将平行光管的十字分划移动至图像中中间区域，左右控制【光栅尺】来调整相机位置,直至平行光管的十字分划线最清晰;\n"
//                   "6、单击【获取图片】,获取相机采集的图片用于后续图像处理;\n"
//                << "Step3:\n"
//                   "1、单击【图像分析】，分析结束后会将分析结果显示在图像显示区域;\n"
//                   "2、在图像上，单击需要检测倾斜度的直线;\n"
//                   "3、单击【指标计算】，自动计算分划倾斜，检测结果显示在【应用程序输出】中\n"
//                   "重复上述试验6次，考核分划倾斜检测的重复性;";
//    }
//    //像倾斜、相对像倾斜检测
//    if(index == 7 || index == 8)
//    {
//        m_steps << "Step2:\n"
//                   "1、产品视度归零;\n"
//                   "2、用铅垂线作为基准，调节摄像机电子分划竖线与铅垂线一致;\n"
//                   "3、水平放置产品;\n"
//                   "4、打开【自动曝光】、【自动增益】;\n"
//                   "5、单击【开始采集】;\n"
//                   "6、将平行光管的十字分划移动至图像中中间区域，左右控制【光栅尺】来调整相机位置,直至平行光管的十字分划线最清晰;\n"
//                   "7、单击【获取图片】\n"
//                << "Step3:\n"
//                   "1、单击【图像分析】，分析结束后会将分析结果显示在图像显示区域;\n"
//                   "2、在图像上，单击需要检测倾斜度的直线;\n"
//                   "3、单击【指标计算】，自动计算分划倾斜，检测结果显示在【应用程序输出】中\n"
//                   "重复上述试验6次，考核像倾斜、相对像倾斜检测的重复性;";
//    }
//    //光轴平行度检测
//    if(index == 9)
//    {
//        m_steps << "Step2:\n"
//                   "1、陪试品（望远镜）视度归零;\n"
//                   "2、利用转像棱镜;\n"
//                   "3、打开【自动曝光】、【自动增益】;\n"
//                   "4、单击【开始采集】;\n"
//                   "5、将平行光管的十字分划移动至图像中中间区域，左右控制【光栅尺】来调整相机位置,直至平行光管的十字分划线最清晰;\n"
//                   "6、利用转像棱镜，用摄像机观察产品另一侧镜筒中平行光管分划中心的像，左右控制【光栅尺】直至平行光管的十字分划线最清晰;\n"
//                   "7、单击【获取图片】\n"
//                << "Step3:\n"
//                   "1、单击【图像分析】，分析结束后会将分析结果显示在图像显示区域;\n"
//                   "2、在显示图片上，框选测算光轴平行度的十字中心区域;\n"
//                   "3、单击【指标计算】，自动计算光轴平行度，检测结果显示在【应用程序输出】中;\n"
//                   "水平方向上，检测结果大于零则发散，小于零则会聚;\n"
//                   "重复上述试验6次，考核光轴平行度检测的重复性;";
//    }
//    //左右分辨力检测
//    if(index == 10 || index == 11)
//    {
//        m_steps << "Step2:\n"
//                   "1、打开【自动曝光】、【自动增益】;\n"
//                   "2、单击【开始采集】;\n"
//                   "3、将平行光管的十字分划移动至图像中中间区域，左右控制【光栅尺】来调整相机位置,直至平行光管的十字分划线最清晰;\n"
//                   "4、通过摄像机分别观察产品左、右镜筒中分辨力板的图案，4名观察者中3名刚能分辨4个方向图案对应的组数作为产品的分辨力测量值，分别观测记录左、右镜筒的分辨力值;\n"
//                   "5、观察结束后，单击【停止采集】;\n"
//                << "Step3:\n"
//                   "1、观察者观察结束后先下拉选择分辨力板类型，输入【单元号】，以回车键结束输入，检测结果显示在【应用程序输出】中\n"
//                   "重复上述试验6次，考核分辨力检测的重复性;";
//    }
//    //左右出瞳直径检测
//    if(index == 12 || index == 13)
//    {
//        m_steps << "Step2:\n"
//                   "a)将陪试品（望远镜）视度归零;\n"
//                   "b)前后移动摄像机直到看清孔径光栏为止，测量记录其直径为出瞳直径;\n"
//                   "c)检测结束后，输入【检测结果】，检测结果显示在【应用程序输出】中\n"
//                   "重复上述试验6次，考核出瞳直径检测的重复性;";
//    }
//    //瞳距适应范围检测
//    if(index == 14)
//    {
//        m_steps << "Step2:\n"
//                   "a)摄像机需左右平移;\n"
//                   "b)调节陪试品（望远镜）瞳距，通过摄像机及显微镜头测量陪试品（望远镜）左、右镜筒的出瞳中心，记录最大值和最小值;\n"
//                   "c)最大值和最小值的差为瞳距适应范围;\n"
//                   "d)检测结束后，输入【检测结果】，检测结果显示在【应用程序输出】中\n"
//                   "重复上述试验6次，考核瞳距适应范围检测的重复性;";
//    }
//    //左右出瞳距离检测
//    if(index == 15 || index == 16)
//    {
//        m_steps << "Step2:\n"
//                   "a)用摄像机看清孔径光阑;\n"
//                   "b)用摄像机看清外面表玻璃（油脂）;\n"
//                   "c)摄像机沿光轴方向移动的距离就是出瞳距离;\n"
//                   "d)检测结束后，输入【检测结果】，检测结果显示在【应用程序输出】中\n"
//                   "重复上述试验6次，考核出瞳距离检测的重复性;";
//    }
//    //左右倍率检测
//    if(index == 17 || index == 18 )
//    {
//        m_steps << "Step2:\n"
//                   "a)陪试品（望远镜）视度归零;\n"
//                   "b)用标准光栏测量，将标准光栏分别置于陪试品（望远镜）左、右支物镜前，使光栏中心与产品光轴基本重合;\n"
//                   "c)前后移动摄像机至能清晰观察到标准光阑的像;\n"
//                   "d)用游标卡尺测量标准光栏直径（两分划间距离;\n"
//                   "e)用摄像机上的电子分化测量对应的标准光栏的像的直径（两分划间距离）;\n"
//                   "f)倍率=标准光栏的像的直径（两分划间距离）/标准光栏直径（两分划间距离）;\n"
//                   "g)检测结束后，输入【检测结果】，检测结果显示在【应用程序输出】中\n"
//                   "重复上述试验6次，考核倍率检测的重复性;";
//    }
//    //倍率差检测
//    if(index == 19)
//    {
//        m_steps << "Step2:\n"
//                   "a)分别测算出左、右镜筒倍率（测量6次的平均值），按公式计算倍率差;\n"
//                   "b)检测结束后，输入【检测结果】，检测结果显示在【应用程序输出】中\n"
//                   "重复上述试验6次，考核倍率差检测的重复性;";
//    }
//    //左右视场检测
//    if(index == 20 || index == 21)
//    {
//        m_steps << "Step2:\n"
//                   "a)将陪试品（望远镜）放在宽角准直仪物镜前，分别调整陪试品（望远镜）使左、右镜筒的视场中心与宽角准直仪中心重合;\n"
//                   "b)前后移动摄像机至能清晰观察宽角准直仪分划，观察读取摄像机中看到的宽角准直仪左右边缘的读数，即为视场;\n"
//                   "c)检测结束后，输入【检测结果】，检测结果显示在【应用程序输出】中\n"
//                   "重复上述试验6次，考核视场检测的重复性;";
//    }
//    //左右畸变检测
//    if(index == 22 || index == 23)
//    {
//                   m_steps << "Step2:\n"
//                              "a)将陪试品（望远镜）放在宽角准直仪物镜前，分别调整陪试品（望远镜）使左、右镜筒的视场中心与宽角准直仪中心重合;\n"
//                              "b)摄像机电子分划对准陪试品（望远镜）视场中心;\n"
//                              "c)在宽角准直仪中选择陪试品（望远镜）最大视场α的分划刻线标识;\n"
//                              "d)摄像机电子分划由视场中心向最大视场α的分划刻线方向转动，使摄像机电子分划对准最大视场α的分划刻线并看清;\n"
//                              "e)读取此时该标识与产品视场中心的角度β（编码器角度值）,按公式计算畸变;\n"
//                              "f)检测结束后，输入【检测结果】，检测结果显示在【应用程序输出】中\n"
//                              "重复上述试验6次，考核畸变检测的重复性;";
//    }
    m_steps = this->stepTipsMap[index];
    this->stepCount = m_steps.size();
    ui->progressBarWidiget->setStepCount(this->stepCount);
    ui->progressBarWidiget->setStepTexts(m_steps);
    ui->progressBarWidiget->setStartFlag(true);
    myProgressBarStepUpdate(0);
    this->step = 0;//设置当前步骤为第0步
}
void MainWindow::loadStepTipsDoc(){
    QFile file(QCoreApplication::applicationDirPath()+"/doc/stepTips.txt");
    QStringList stepsList;
    int itemIndex = 0;
    QString str;
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"error opening the text of stepTips";
        return ;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    while(!stream.atEnd()){
        QString line = stream.readLine();
        if(line.isEmpty()){//空行为项目的标识
            if(!str.isEmpty()){
                stepsList.append(str);
            }
            if(!stepsList.isEmpty()){
                stepTipsMap.insert(std::make_pair(itemIndex,stepsList));
            }
            ++itemIndex;
            stepsList.clear();
            str.clear();
            continue;
        }
        if(line.startsWith("@")){//@为每步的标识
            if(!str.isEmpty()){
                stepsList.append(str);
                str.clear();
            }
            line = line.mid(1);
        }
        if(str.isEmpty()){
            str += line;
        }
        else
            str = str + "\n" +line;
    }
    if(!str.isEmpty()){
        stepsList.append(str);
        stepTipsMap.insert(std::make_pair(itemIndex,stepsList));
    }
}

void MainWindow::myProgressBarStepUpdate(int currentStep)
{
    int temp = currentStep;
    if(currentStep>=0){
        temp = currentStep % (this->stepCount + 1);
    }
    else
        temp = 0;
    ui->progressBarWidiget->setCurrentStep(temp);
    this->step = temp ;
}

void MainWindow::onTimeout()
{
    this->timeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    //更新曝光时间
    if(this->isAutoExposureTime)
    {
        float exposureTime;
        this->m_mvCamera->GetExposureTime(&exposureTime);
        ui->ExposureLineEdit->setText(QString::number(exposureTime));
        ui->ExposureLineEdit_3->setText(QString::number(exposureTime));
    }
    //更新增益
    if(this->isAutoGain)
    {
        float gain;
        this->m_mvCamera->GetGain(&gain);
        ui->GainLineEdit->setText(QString::number(gain));
        ui->GainLineEdit_3->setText(QString::number(gain));
    }
    //更新离焦距离值
    this->defocusDistance = -this->udpThread->distance + this->defocusDistanceZero;
    ui->ruleDistanceLineEdit->setText(QString::number(this->defocusDistance));
    //设置scrollBar位置(1:1)
    if(isChecked){
        int offsetX = ui->displayLabel->width()/2-ui->scrollArea->width()/2;
        int offsetY = ui->displayLabel->height()/2-ui->scrollArea->height()/2;
        this->setScrollBarWHOffset(offsetX,offsetY);
        isChecked = false;
    }
    //扩大图像显示面积
    if(this->isClickToolBar){
        ui->displayLabel->setFixedSize(ui->scrollArea->width(),ui->scrollArea->height());
        ui->checkBox->setChecked(false);
        this->isClickToolBar = false;
    }

}

bool MainWindow::saveImage(cv::Mat& image)
{
    cv::Mat image_rgb ;
    cv::cvtColor(image,image_rgb,cv::COLOR_BGR2RGB);
    QImage img((uchar *)image_rgb.data,image_rgb.cols,image_rgb.rows,image_rgb.cols * 3,QImage::Format_RGB888);
    QDir dir;
    QString dirPath = QString("./assets/%1").arg(this->productID);
    if(dir.mkpath(dirPath))
    {   //图片格式是png会出错，估计是压缩了
        QString save_path = QDir(dirPath).path()+"/"+QString("%1_%2.%3").arg(this->productID).arg(this->itemID[this->finishItemCount]).arg(this->saveFormat);
        img.save(save_path,const_cast<const char*>((char*)this->saveFormat.toUtf8().data()),100);
        QString abso_path = QDir::toNativeSeparators(QDir(save_path).absolutePath());
        QString imgUrl = QUrl::fromLocalFile(abso_path).toString();
        qDebug()<<imgUrl;
        return true;
    }
    else
        return false;

}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_mvCamera;
    delete m_getImageThread;
    delete myDatabase;
    delete p;
    delete myProgressBar;
    delete separator;
    delete splitter;
    delete timeLabel;
    delete myTimer;
    delete m_camDeviceList;


}

//显示数据库界面
void MainWindow::on_action_triggered()
{
    OperatorDatabaseWidget *w = new OperatorDatabaseWidget(this);
    w->setWindowIcon(QIcon(":/frameless_images/logo.png"));
    w->setAttribute(Qt::WA_DeleteOnClose);  //close后直接delete
    w->show();
}
//状态栏中隐藏tabwidget_2
void MainWindow::on_action_2_triggered()
{
    if(!ui->tabWidget_2->isHidden()){
        if(ui->action_3->isChecked()){
            ui->tabWidget_2->setCurrentIndex(0);
            ui->action_2->setChecked(true);
            ui->action_3->setChecked(false);
        }
        else{
            ui->tabWidget_2->hide();
            ui->action_2->setChecked(false);
            ui->action_3->setChecked(false);
        }
    }
    else{
        ui->tabWidget_2->setCurrentIndex(0);
        ui->tabWidget_2->show();
        ui->action_2->setChecked(true);
        ui->action_3->setChecked(false);
    }
    this->isClickToolBar = true;
}

void MainWindow::on_action_3_triggered()
{
    if(!ui->tabWidget_2->isHidden()){
        if(ui->action_2->isChecked()){
            ui->tabWidget_2->setCurrentIndex(1);
            ui->action_3->setChecked(true);
            ui->action_2->setChecked(false);
        }
        else{
            ui->tabWidget_2->hide();
            ui->action_2->setChecked(false);
            ui->action_3->setChecked(false);
        }
    }
    else{
        ui->tabWidget_2->setCurrentIndex(1);
        ui->tabWidget_2->show();
        ui->action_3->setChecked(true);
        ui->action_2->setChecked(false);
    }
    this->isClickToolBar = true;
}


void MainWindow::onEnumButtonReleased()
{
    //清空combox
    ui->comboBox->clear();
    delete  this->m_camDeviceList;
    this->m_camDeviceList = new MV_CC_DEVICE_INFO_LIST;
    int nRet = m_mvCamera->EnumDevices(m_camDeviceList);

    // 找到设备，处理流程
    if( nRet == MV_OK)
    {
        // 更新UI交互
        // 更新 comboBox
        for (unsigned int i = 0; i < m_camDeviceList->nDeviceNum; i++)
        {
            qDebug("[device %d]:\n", i);
            MV_CC_DEVICE_INFO *pDeviceInfo = m_camDeviceList->pDeviceInfo[i];
            if (nullptr == pDeviceInfo)
            {
                QMessageBox::warning(this,"枚举提示","枚举出一个空设备");
                continue;
            }
            if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
            {
                int nIp1 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
                int nIp2 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
                int nIp3 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
                int nIp4 =  (pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);
                QString deviceName = QString::number(nIp1) + "." + QString::number(nIp2) + "." + QString::number(nIp3) + "." + QString::number(nIp4);
                ui->comboBox->addItem(deviceName);
            }
            // 在 comboBox 中加入信息
        }
        // 弹窗提示
//        QMessageBox::information(this, "OK", "查找设备成功！");
        // 更新 openButton
        ui->openButton->setEnabled(true);
    }

    // 没有发现设备，报错
    if( nRet == (int)MV_NO_DEVICE_FOUND )
        QMessageBox::information(this, "异常", "没有发现设备！");
}

void MainWindow::onOpenButtonReleased()
{
    int deviceIndex = ui->comboBox->currentIndex();
    // TODO: 选择相机的指引异常处理
    int nRet = m_mvCamera->Open(m_camDeviceList->pDeviceInfo[deviceIndex]);

    if ( nRet == MV_OK)
    {
        // 更新UI
        // 取消使能
        ui->openButton->setDisabled(true);
        ui->enumButton->setEnabled(false);
        // 使能
        ui->closeButton->setEnabled(true);
        ui->startGetImageButton->setEnabled(true);
        ui->ExposureLineEdit->setEnabled(true);
        ui->GainLineEdit->setEnabled(true);
        ui->AcquisitionFrameRateLineEdit->setEnabled(true);
        ui->checkBox_2->setEnabled(true);
        ui->checkBox_3->setEnabled(true);
        ui->ExposureLineEdit_3->setEnabled(true);
        ui->GainLineEdit_3->setEnabled(true);
        ui->AcquisitionFrameRateLineEdit_3->setEnabled(true);
        ui->checkBox_4->setEnabled(true);
        ui->checkBox_5->setEnabled(true);
        //相机参数初始化
        this->m_mvCamera->CloseExposureTimeAuto();
        this->m_mvCamera->CloseGainAuto();
        this->m_mvCamera->SetExposureTime(800);
        this->m_mvCamera->SetGain(4);
        this->m_mvCamera->SetAcquisitionFrameRate(10);
        on_GetParamButton_clicked();
    }

    if( nRet != MV_OK)
    {
        QMessageBox::information(this, "异常", "打开设备异常！");
    }
}

void MainWindow::onCloseButtonReleased()
{
    //关闭采集
    // 关闭采集进程
    m_getImageThread->setGetImageThreadFlag(false);
    m_getImageThread->wait();  // 等待线程结束; wait()函数在 5.15 中引入
    m_getImageThread->exit(0); // 等价于 quit()
    m_mvCamera->StopGrabbing();

    int nRet = m_mvCamera->Close();

    if( nRet == MV_OK )
    {
        // 更新UI
        // 取消使能
        ui->closeButton->setDisabled(true);
        ui->startGetImageButton->setDisabled(true);
        ui->stopGetImageButton->setEnabled(false);
        // 使能
        ui->openButton->setEnabled(true);
        ui->enumButton->setEnabled(true);
        //使能指标界面
        ui->getImageButton->setEnabled(false);
        if(!ui->productIDLineEdit1->text().isEmpty())
        {
            ui->startGrabButton->setEnabled(true);
        }

        ui->ExposureLineEdit->setEnabled(false);
        ui->GainLineEdit->setEnabled(false);
        ui->AcquisitionFrameRateLineEdit->setEnabled(false);
        ui->checkBox_2->setChecked(false);
        ui->checkBox_3->setChecked(false);
        ui->checkBox_2->setEnabled(false);
        ui->checkBox_3->setEnabled(false);
        ui->ExposureLineEdit_3->setEnabled(false);
        ui->GainLineEdit_3->setEnabled(false);
        ui->AcquisitionFrameRateLineEdit_3->setEnabled(false);
        ui->checkBox_4->setChecked(false);
        ui->checkBox_5->setChecked(false);
        ui->checkBox_4->setEnabled(false);
        ui->checkBox_5->setEnabled(false);
        this->isAutoExposureTime = false;
        this->isAutoGain = false;

    }

    if(nRet != MV_OK)
    {
        QMessageBox::information(this, "异常", "设备关闭失败");
    }
}

void MainWindow::onstartGetImageButtonReleased()
{
    // 调用相机库，开始流抓取
    int nRet = m_mvCamera->StartGrabbing();

    if(nRet == MV_OK)
    {
        // 创建 设置 启动 图片抓取线程
        m_getImageThread->setGetImageThreadFlag(true);
        m_getImageThread->start();

        // 更新UI
        // - 取消使能
        ui->startGetImageButton->setDisabled(true);
        // - 使能
        ui->stopGetImageButton->setEnabled(true);
    }

    if(nRet != MV_OK)
    {
        QMessageBox::information(this, "异常", "图像抓取失败！");
    }
}

void MainWindow::onstopGetImageButtonReleased()
{
    // 关闭 图片抓取线程
    m_getImageThread->setGetImageThreadFlag(false);
    m_getImageThread->wait();  // 等待线程结束; wait()函数在 5.15 中引入
    m_getImageThread->exit(0); // 等价于 quit()

    // 调用相机库，取消流抓取
    int nRet = m_mvCamera->StopGrabbing();

    if(nRet == MV_OK)
    {
        // 更新UI
        // 使能
        ui->startGetImageButton->setEnabled(true);
        ui->closeButton->setEnabled(true);
        // 取消使能
        ui->stopGetImageButton->setDisabled(true);
        qDebug()<<"停止采集成功"<<endl;
    }
    else
//    if(nRet != MV_OK)
    {
        QMessageBox::information(this, "异常", "停止采集失败！");
    }
}


void MainWindow::updateDisplayLabel(MV_FRAME_OUT newFrame)
{
    // 以下仅能取一种使用

    // 方法一： 调用海康的显示方法  窗口嵌套后不显示图像，而且考虑到后期保存图片，所以不推荐这种方法
//    MV_DISPLAY_FRAME_INFO stDisplayInfo;
//    stDisplayInfo.hWnd = (void *)ui->displayLabel->winId();
//    stDisplayInfo.pData = newFrame.pBufAddr;
//    stDisplayInfo.nDataLen = newFrame.stFrameInfo.nFrameLen;
//    stDisplayInfo.nWidth = newFrame.stFrameInfo.nWidth;
//    stDisplayInfo.nHeight = newFrame.stFrameInfo.nHeight;
//    stDisplayInfo.enPixelType = newFrame.stFrameInfo.enPixelType;
//    qDebug()<<"显示图片"<<endl;
//    m_mvCamera->DisplayOneFrame(&stDisplayInfo);

    /*方法二： 提取图片信息，在Label上显示图片
     * 方法二有三种方法：
     * 1、调用海康的像素转换接口，但是显示的图片后有黑屏的情况
     * 2、直接将采集的pdata数据构造Mat，这种方法虽然显示的照片看不出来变化，但是会导致图像失真
     * 3、调用opencv cvtcolor将像素格式转换，值得推荐
     */
//    m_mutex.lock();
    //调用海康接口转换像素格式
//    this->m_mvCamera->ConvertPixelTypeToGray(&newFrame,image_GRAY);
//    this->m_mvCamera->ConvertPixelTypeToRGB(&newFrame,image_RGB);
    //直接使用原像素格式转成灰度，但是这样会导致失真
//    cv::Mat getImage = cv::Mat(newFrame.stFrameInfo.nHeight,newFrame.stFrameInfo.nWidth,CV_8UC1,newFrame.pBufAddr);
//    getImage.copyTo(image_GRAY);

    try
    {
        //cvtcolor 转换像素格式
        cv::Mat bayerMat(newFrame.stFrameInfo.nHeight,newFrame.stFrameInfo.nWidth,CV_8UC1,newFrame.pBufAddr);
        this->image_GRAY = cv::Mat(newFrame.stFrameInfo.nHeight,newFrame.stFrameInfo.nWidth,CV_8UC1);
        cv::cvtColor(bayerMat,image_GRAY,cv::COLOR_BayerRG2GRAY);
        this->image_BGR = cv::Mat(newFrame.stFrameInfo.nHeight,newFrame.stFrameInfo.nWidth,CV_8UC3);
        cv::cvtColor(bayerMat,image_BGR,cv::COLOR_BayerRG2BGR);
        this->image_BGR_display = image_BGR.clone();
        //绘制电子分划
        p->paintElectronicCrossLine(image_BGR_display);
        ui->displayLabel->clear();
        ui->displayLabel->setPixmap(QPixmap::fromImage(p->cvMat2QImage(image_BGR_display)));
        //计算图像清晰度
        float sharpness = p->imageSharpness(image_GRAY,ui->displayLabel->rectArea);
        ui->sharpnessLineEdit->setText(QString::number(sharpness));
        ui->sharpnessLineEdit_3->setText(QString::number(sharpness));
    }
    catch (cv::Exception &e)
    {
        QMessageBox::critical(this,"opencv error",e.what(),QMessageBox::Yes);
    }
//    m_mutex.unlock();



}

void MainWindow::on_GetParamButton_clicked()
{
    float exposureTime,gain,frameRate,lframeRate;
    //获取参数
    this->m_mvCamera->GetExposureTime(&exposureTime);
    this->m_mvCamera->GetGain(&gain);
    this->m_mvCamera->GetAcquisitionFrameRate(&frameRate);
    this->m_mvCamera->GetResultingFrameRate(&lframeRate);
    //显示参数
    ui->ExposureLineEdit->setText(QString::number(exposureTime));
    ui->GainLineEdit->setText(QString::number(gain));
    ui->AcquisitionFrameRateLineEdit->setText(QString::number(frameRate));
    ui->FrameRateLineEdit->setText(QString::number(lframeRate));
    ui->ExposureLineEdit_3->setText(QString::number(exposureTime));
    ui->GainLineEdit_3->setText(QString::number(gain));
    ui->AcquisitionFrameRateLineEdit_3->setText(QString::number(frameRate));
    ui->FrameRateLineEdit_3->setText(QString::number(lframeRate));
}

void MainWindow::on_ExposureLineEdit_returnPressed()
{
    if(this->m_mvCamera->SetExposureTime(ui->ExposureLineEdit->text().toFloat()) != MV_OK)
    {
        QMessageBox::warning(this,"错误","设置曝光失败！",QMessageBox::Yes);
    }
    on_GetParamButton_clicked();
}

void MainWindow::on_GainLineEdit_returnPressed()
{
    if(this->m_mvCamera->SetGain(ui->GainLineEdit->text().toFloat()) != MV_OK)
    {
        QMessageBox::warning(this,"error","设置增益失败，增益值应小于20！",QMessageBox::Yes);
    }
    on_GetParamButton_clicked();
}

void MainWindow::on_AcquisitionFrameRateLineEdit_returnPressed()
{
    if(this->m_mvCamera->SetAcquisitionFrameRate(ui->AcquisitionFrameRateLineEdit->text().toFloat()) != MV_OK)
    {
        QMessageBox::warning(this,"error","设置帧率失败，帧率应不小于0.5",QMessageBox::Yes);
    }
    on_GetParamButton_clicked();
}

void MainWindow::on_ExposureLineEdit_3_returnPressed()
{
    if(this->m_mvCamera->SetExposureTime(ui->ExposureLineEdit_3->text().toFloat()) != MV_OK)
    {
        QMessageBox::warning(this,"error","设置曝光失败！",QMessageBox::Yes);
    }
    on_GetParamButton_clicked();
}

void MainWindow::on_GainLineEdit_3_returnPressed()
{
    if(this->m_mvCamera->SetGain(ui->GainLineEdit_3->text().toFloat()) != MV_OK)
    {
        QMessageBox::warning(this,"error","设置增益失败，增益值应小于20！",QMessageBox::Yes);
    }
    on_GetParamButton_clicked();
}

void MainWindow::on_AcquisitionFrameRateLineEdit_3_returnPressed()
{
    if(this->m_mvCamera->SetAcquisitionFrameRate(ui->AcquisitionFrameRateLineEdit_3->text().toFloat()) != MV_OK)
    {
        QMessageBox::warning(this,"error","设置帧率失败，帧率应不小于0.5",QMessageBox::Yes);
    }
    on_GetParamButton_clicked();
}

void MainWindow::on_SaveBMPButton_clicked()
{
    this->saveFormat = "BMP";
    ui->plainTextEdit->appendPlainText(QString("设置图片保存格式为BMP"));
}

void MainWindow::on_SaveJPGButton_clicked()
{
    this->saveFormat = "JPG";
    ui->plainTextEdit->appendPlainText(QString("设置图片保存格式为JPG"));
}

void MainWindow::saveSettings()
{
    QSettings settings("nanjing university of technology and science ","TDS");
    //保存结果变量
    settings.setValue("productID",ui->productIDLineEdit->text());
    settings.setValue("gradLean",ui->gradLeanLineEdit->text());
    settings.setValue("leftImageIncline",ui->leftImageInclineLineEdit->text());
    settings.setValue("rightImageIncline",ui->rightImageInclineLineEdit->text());
    settings.setValue("relImageIncline",ui->relImageInclineLineEdit->text());
    settings.setValue("hAxisParalDegree",ui->hAxisParalDegreeLineEdit->text());
    settings.setValue("vAxisParalDegree",ui->vAxisParalDegreeLineEdit->text());
    settings.setValue("leftParal",ui->leftParalLineEdit->text());
    settings.setValue("rightParal",ui->rightParalLineEdit->text());
    settings.setValue("leftAdjustRange",ui->leftAdjustRangeLineEdit->text());
    settings.setValue("rightAdjustRange",ui->rightAdjustRangeLineEdit->text());
    settings.setValue("leftVisZero",ui->leftVisZeroLineEdit->text());
    settings.setValue("rightVisZero",ui->rightVisZeroLineEdit->text());
    settings.setValue("leftResolution",ui->leftResolutionLneEdit->text());
    settings.setValue("rightResolution",ui->rightResolutionLineEdit->text());
    settings.setValue("leftField",ui->leftFieldLineEdit->text());
    settings.setValue("rightField",ui->rightFieldLineEdit->text());
    settings.setValue("leftDistortion",ui->leftDistortionLineEdit->text());
    settings.setValue("rightDistortion",ui->rightDistortionLineEdit->text());
    settings.setValue("leftExitPupil",ui->leftExitPupilLineEdit->text());
    settings.setValue("rightExitPupil",ui->rightExitPupilLineEdit->text());
    settings.setValue("leftExitPupilDistance",ui->leftExitPupilDistanceLineEdit->text());
    settings.setValue("rightExitPupilDistance",ui->rightExitPupilDistanceLineEdit->text());
    settings.setValue("leftRate",ui->leftRateLineEdit->text());
    settings.setValue("rightRate",ui->rightRateLineEdit->text());
    settings.setValue("rateDif",ui->rateDifLineEdit->text());
    settings.setValue("pupilDistanceRange",ui->pupilDistanceRangeLineEdit->text());
    //保存过程变量
//    settings.setValue("ndetectResult",ndetectResult);
    settings.setValue("aveDetectResult",aveDetectResult);
//    settings.setValue("ndetectResult_vaxis",ndetectResult_vaxis);
    settings.setValue("aveDetectResult_vaxis",aveDetectResult_vaxis);
    settings.setValue("nCurrentCount",nCurrentCount);
    settings.setValue("currentItemIndex",currentItemIndex);
    settings.setValue("currentProductType",currentProductType);
    settings.setValue("finishItemCount",finishItemCount);
    QVariantList list;
    for(int i=0; i<this->finishItemFlags.size(); i++){
        list.append(QVariant(finishItemFlags[i]));
    }
    settings.setValue("finishItemFlags",list);
    QVariantList leftImageInclineList;
    for(int i = 0;i<6;++i){
        leftImageInclineList.append(QVariant(leftImageInclineArray[i]));
    }
    settings.setValue("leftImageInclineArray",leftImageInclineList);
    QVariantList rightImageInclineList;
    for(int i = 0;i<6;++i){
        rightImageInclineList.append(QVariant(rightImageInclineArray[i]));
    }
    settings.setValue("rightImageInclineArray",rightImageInclineList);

    QVariantList leftRatelist;
    for(int i = 0;i<6;++i){
        leftRatelist.append(QVariant(leftRateArray[i]));
    }
    settings.setValue("leftRateArray",leftRatelist);
    QVariantList rightRateList;
    for(int i = 0;i<6;++i){
        rightRateList.append(QVariant(rightRateArray[i]));
    }
    settings.setValue("rightRateArray",rightRateList);

    QVariantList leftViszerolist;
    for(int i = 0;i<6;++i){
        leftViszerolist.append(QVariant(leftVisZeroArray[i]));
    }
    settings.setValue("leftVisZeroArray",leftViszerolist);
    QVariantList rightVisZeroList;
    for(int i = 0;i<6;++i){
        rightVisZeroList.append(QVariant(rightVisZeroArray[i]));
    }
    settings.setValue("rightVisZeroArray",rightVisZeroList);
    //保存参数设置
    settings.setValue("pExitPupilDistance",pExitPupilDistance);
    settings.setValue("pMagnification",pMagnification);
    settings.setValue("lenFocus",lenFocus);
    settings.setValue("pP2",pP2);
    settings.setValue("defocusDistanceZero",defocusDistanceZero);
    qDebug()<<"[info] 保存状态";
}

void MainWindow::loadSettings()
{
    QSettings settings("nanjing university of technology and science ","TDS");
    //加载结果变量
    this->finishItemCount = settings.value("finishItemCount").toInt();
    QList<QVariant> list = settings.value("finishItemFlags").toList();
    for(int i = 0;i<list.size();++i)
    {
        this->finishItemFlags[i] = list[i].toBool();
    }
    this->productID = settings.value("productID").toString();
    ui->productIDLineEdit->setText(this->productID);
    this->gradLean = settings.value("gradLean").toString();
    if(finishItemFlags[6]){
        ui->gradLeanLineEdit->setText(this->gradLean);
    }

    this->leftImageIncline = settings.value("leftImageIncline").toString();
    if(finishItemFlags[7]){
        ui->leftImageInclineLineEdit->setText(this->leftImageIncline);
    }

    this->rightImageIncline = settings.value("rightImageIncline").toString();
    if(finishItemFlags[8]){
        ui->rightImageInclineLineEdit->setText(this->rightImageIncline);
    }

    this->relImageIncline = settings.value("relImageIncline").toString();
    if(finishItemFlags[9]){
        ui->relImageInclineLineEdit->setText(this->relImageIncline);
    }

    this->hAxisParalDegree = settings.value("hAxisParalDegree").toString();
    if(finishItemFlags[10]){
        ui->hAxisParalDegreeLineEdit->setText(this->hAxisParalDegree);
    }

    this->vAxisParalDegree = settings.value("vAxisParalDegree").toString();
    if(finishItemFlags[10]){
        ui->vAxisParalDegreeLineEdit->setText(this->vAxisParalDegree);
    }

    this->leftParal = settings.value("leftParal").toString();
    if(finishItemFlags[0]){
        ui->leftParalLineEdit->setText(this->leftParal);
    }

    this->rightParal = settings.value("rightParal").toString();
    if(finishItemFlags[1]){
        ui->rightParalLineEdit->setText(this->rightParal);
    }

    this->leftAdjustRange = settings.value("leftAdjustRange").toString();
    if(finishItemFlags[2]){
        ui->leftAdjustRangeLineEdit->setText(this->leftAdjustRange);
    }

    this->rightAdjustRange = settings.value("rightAdjustRange").toString();
    if(finishItemFlags[3]){
        ui->rightAdjustRangeLineEdit->setText(this->rightAdjustRange);
    }

    this->leftVisZero = settings.value("leftVisZero").toString();
    if(finishItemFlags[4]){
        ui->leftVisZeroLineEdit->setText(this->leftVisZero);
    }

    this->rightVisZero = settings.value("rightVisZero").toString();
    if(finishItemFlags[5]){
        ui->rightVisZeroLineEdit->setText(this->rightVisZero);
    }

    this->leftResolution = settings.value("leftResolution").toString();
    if(finishItemFlags[11]){
        ui->leftResolutionLneEdit->setText(this->leftResolution);
    }

    this->rightResolution = settings.value("rightResolution").toString();
    if(finishItemFlags[12]){
        ui->rightResolutionLineEdit->setText(this->rightResolution);
    }

    this->leftField = settings.value("leftField").toString();
    if(finishItemFlags[21]){
        ui->leftFieldLineEdit->setText(this->leftField);
    }

    this->rightField = settings.value("rightField").toString();
    if(finishItemFlags[22]){
        ui->rightFieldLineEdit->setText(this->rightField);
    }

    this->leftDistortion = settings.value("leftDistortion").toString();
    if(finishItemFlags[23]){
        ui->leftDistortionLineEdit->setText(this->leftDistortion);
    }

    this->rightDistortion = settings.value("rightDistortion").toString();
    if(finishItemFlags[24]){
        ui->rightDistortionLineEdit->setText(this->rightDistortion);
    }

    this->leftExitPupil = settings.value("leftExitPupil").toString();
    if(finishItemFlags[13]){
        ui->leftExitPupilLineEdit->setText(this->leftExitPupil);
    }

    this->rightExitPupil = settings.value("rightExitPupil").toString();
    if(finishItemFlags[14]){
        ui->rightExitPupilLineEdit->setText(this->rightExitPupil);
    }

    this->leftExitPupilDistance = settings.value("leftExitPupilDistance").toString();
    if(finishItemFlags[16]){
        ui->leftExitPupilDistanceLineEdit->setText(this->leftExitPupilDistance);
    }

    this->rightExitPupilDistance = settings.value("rightExitPupilDistance").toString();
    if(finishItemFlags[17]){
        ui->rightExitPupilDistanceLineEdit->setText(this->rightExitPupilDistance);
    }

    this->leftRate = settings.value("leftRate").toString();
    if(finishItemFlags[18]){
        ui->leftRateLineEdit->setText(this->leftRate);
    }

    this->rightRate = settings.value("rightRate").toString();
    if(finishItemFlags[19]){
        ui->rightRateLineEdit->setText(this->rightRate);
    }

    this->rateDif = settings.value("rateDif").toString();
    if(finishItemFlags[20]){
        ui->rateDifLineEdit->setText(this->rateDif);
    }

    this->pupilDistanceRange = settings.value("pupilDistanceRange").toString();
    if(finishItemFlags[15]){
        ui->pupilDistanceRangeLineEdit->setText(this->pupilDistanceRange);
    }

    //加载过程变量
//    this->ndetectResult = settings.value("ndetectResult").toDouble();
    this->aveDetectResult = settings.value("aveDetectResult").toDouble();
//    this->ndetectResult_vaxis = settings.value("ndetectResult_vaxis").toDouble();
    this->aveDetectResult_vaxis = settings.value("aveDetectResult_vaxis").toDouble();
    this->nCurrentCount = settings.value("nCurrentCount").toInt();
    this->currentItemIndex = settings.value("currentItemIndex").toInt();
    this->currentProductType = settings.value("currentProductType").toInt();
    QList<QVariant> list1 = settings.value("leftImageInclineArray").toList();
    for(int i = 0;i<list1.size();++i)
    {
        this->leftImageInclineArray[i] = list1[i].toDouble();
    }
    QList<QVariant> list2 = settings.value("rightImageInclineArray").toList();
    for(int i = 0;i<list2.size();++i)
    {
        this->rightImageInclineArray[i] = list2[i].toDouble();
    }
    QList<QVariant> list3 = settings.value("leftRateArray").toList();
    for(int i = 0;i<list3.size();++i)
    {
        this->leftRateArray[i] = list3[i].toDouble();
    }
    QList<QVariant> list4 = settings.value("rightRateArray").toList();
    for(int i = 0;i<list4.size();++i)
    {
        this->rightRateArray[i] = list4[i].toDouble();
    }
    QList<QVariant> list5 = settings.value("leftVisZeroArray").toList();
    for(int i = 0;i<list5.size();++i)
    {
        this->leftVisZeroArray[i] = list5[i].toDouble();
    }
    QList<QVariant> list6 = settings.value("rightVisZeroArray").toList();
    for(int i = 0;i<list6.size();++i)
    {
        this->rightVisZeroArray[i] = list6[i].toDouble();
    }
    //加载参数设置
    this->pExitPupilDistance = settings.value("pExitPupilDistance").toDouble();
    this->pMagnification = settings.value("pMagnification").toDouble();
    this->lenFocus = settings.value("lenFocus").toDouble();
    this->pP2 = settings.value("pP2").toDouble();
    this->defocusDistanceZero = settings.value("defocusDistanceZero").toDouble();
    ui->PDLineEdit_36->setText(QString::number(this->pExitPupilDistance));
    ui->rateLineEdit_5->setText(QString::number(this->pMagnification));
    ui->focusLineEdit_6->setText(QString::number(this->lenFocus));
    ui->P2LineEdit_7->setText(QString::number(this->pP2));

    //加载产品编号
    ui->productIDLineEdit1->setText((this->productID));
    //布局
    if(this->nCurrentCount != 0)
    {
        itemLayout(this->currentItemIndex);
        //进度条step
        if(currentItemIndex == 0 || currentItemIndex == 1 || currentItemIndex == 4 || currentItemIndex == 5)
        {
            myProgressBarStepUpdate(2);
        }
        else
            myProgressBarStepUpdate(1);
    }
    else
    {
        itemLayout(0);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(this->finishItemCount != this->itemCount)
    {
        if(!ui->productIDLineEdit->text().isEmpty())
        {
            if(QMessageBox::question(this,"question","检测未完成，是否继续关闭窗口？",QMessageBox::Yes|QMessageBox::No,QMessageBox::No)==QMessageBox::Yes)
            {
                saveSettings();
                qDebug()<<"[info] close window.";
            }
            else
            {
                event->ignore();
                return ;
            }
        }
        else
        {
            saveSettings();
        }
    }
    else
    {
        QMessageBox::warning(this,"提示","当前检测已经完成，请注意存档哦!",QMessageBox::Yes);
        event->ignore();
        return;
    }
    event->accept();
}

void MainWindow::on_saveButton_clicked()
{
    if(ui->productIDLineEdit->text().isEmpty())
    {
        QMessageBox::warning(this,"error","请输入产品编号",QMessageBox::Yes);
//        statusBar()->clearMessage();
        return ;
    }
    statusBar()->showMessage(tr("存档..."));
    ui->testTimeLineEdit->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    this->testTime = QDateTime::currentDateTime();
    QDir dir;
    QString dirPath = QString("./assets/%1").arg(this->productID);
    if(dir.mkpath(dirPath))
    {
       this->saveFilePath = this->productID;
    }
    QSqlQuery query;
    query.exec(QString("select * from result where `产品编号` = %1 order by `id` desc").arg(this->productID));
    int dataCount = 0;
    int id = 0; //最新数据行id
    while(query.next()){
        ++dataCount;
        id = query.value(0).toInt();
    }
    qDebug()<<"[info]数据库中存在该产品的"<<dataCount<<"条数据";
    QSqlTableModel *model = new QSqlTableModel;
    model->setTable("result");
    //数据库中已经存在一条数据则将该条数据作为最新数据行，在表中插入一行与之相同的数据行
    QString str = "";
    for(int i = 1;i<30;++i){
        str += QString(" `%1`,").arg(model->headerData(i,Qt::Horizontal).toString());
    }
    str += QString(" `%1`").arg(model->headerData(30,Qt::Horizontal).toString());
    if(dataCount == 1){
        QString sqlStr = QString("insert into result (%1) select %2 from result where `id`=%3").arg(str).arg(str).arg(id);
        if(query.exec(sqlStr)){
            qDebug()<<"[info] 添加最新行成功";
        }
        else{
            qDebug()<<"[info] 添加最新行失败";
            qDebug()<<sqlStr;
        }
    }
    query.prepare(QString("insert into result (`%1`, `%2`,`%3`,`%4`,`%5`,`%6`, `%7`, `%8`,"
                  " `%9`, `%10`, `%11`, `%12` ,`%13` ,`%14` ,`%15` , `%16` ,"
                  "`%17`, `%18`, `%19` ,`%20` ,`%21`, `%22`, `%23`, `%24`, `%25`, `%26`, `%27`, `%28` ,`%29`,`%30`) "
                  "values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)").arg(model->headerData(1,Qt::Horizontal).toString())
                     .arg(model->headerData(2,Qt::Horizontal).toString()).arg(model->headerData(3,Qt::Horizontal).toString()).arg(model->headerData(4,Qt::Horizontal).toString()).arg(model->headerData(5,Qt::Horizontal).toString())
                     .arg(model->headerData(6,Qt::Horizontal).toString()).arg(model->headerData(7,Qt::Horizontal).toString()).arg(model->headerData(8,Qt::Horizontal).toString()).arg(model->headerData(9,Qt::Horizontal).toString())
                     .arg(model->headerData(10,Qt::Horizontal).toString()).arg(model->headerData(11,Qt::Horizontal).toString()).arg(model->headerData(12,Qt::Horizontal).toString()).arg(model->headerData(13,Qt::Horizontal).toString())
                     .arg(model->headerData(14,Qt::Horizontal).toString()).arg(model->headerData(15,Qt::Horizontal).toString()).arg(model->headerData(16,Qt::Horizontal).toString()).arg(model->headerData(17,Qt::Horizontal).toString())
                     .arg(model->headerData(18,Qt::Horizontal).toString()).arg(model->headerData(19,Qt::Horizontal).toString()).arg(model->headerData(20,Qt::Horizontal).toString()).arg(model->headerData(21,Qt::Horizontal).toString())
                     .arg(model->headerData(22,Qt::Horizontal).toString()).arg(model->headerData(23,Qt::Horizontal).toString()).arg(model->headerData(24,Qt::Horizontal).toString()).arg(model->headerData(25,Qt::Horizontal).toString())
                     .arg(model->headerData(26,Qt::Horizontal).toString()).arg(model->headerData(27,Qt::Horizontal).toString()).arg(model->headerData(28,Qt::Horizontal).toString()).arg(model->headerData(29,Qt::Horizontal).toString()).arg(model->headerData(30,Qt::Horizontal).toString()));
    query.addBindValue(ui->productIDLineEdit->text());
    query.addBindValue(ui->gradLeanLineEdit->text());
    query.addBindValue(ui->leftImageInclineLineEdit->text());
    query.addBindValue(ui->rightImageInclineLineEdit->text());
    query.addBindValue(ui->relImageInclineLineEdit->text());
    query.addBindValue(ui->hAxisParalDegreeLineEdit->text());
    query.addBindValue(ui->vAxisParalDegreeLineEdit->text());
    query.addBindValue(ui->leftParalLineEdit->text());
    query.addBindValue(ui->rightParalLineEdit->text());
    query.addBindValue(ui->leftAdjustRangeLineEdit->text());
    query.addBindValue(ui->rightAdjustRangeLineEdit->text());
    query.addBindValue(ui->leftVisZeroLineEdit->text());
    query.addBindValue(ui->rightVisZeroLineEdit->text());
    query.addBindValue(ui->leftResolutionLneEdit->text());
    query.addBindValue(ui->rightResolutionLineEdit->text());
    query.addBindValue(ui->leftFieldLineEdit->text());
    query.addBindValue(ui->rightFieldLineEdit->text());
    query.addBindValue(ui->leftDistortionLineEdit->text());
    query.addBindValue(ui->rightDistortionLineEdit->text());
    query.addBindValue(ui->leftExitPupilLineEdit->text());
    query.addBindValue(ui->rightExitPupilLineEdit->text());
    query.addBindValue(ui->leftExitPupilDistanceLineEdit->text());
    query.addBindValue(ui->rightExitPupilDistanceLineEdit->text());
    query.addBindValue(ui->leftRateLineEdit->text());
    query.addBindValue(ui->rightRateLineEdit->text());
    query.addBindValue(ui->rateDifLineEdit->text());
    query.addBindValue(ui->pupilDistanceRangeLineEdit->text());
    query.addBindValue(this->saveFilePath);
    query.addBindValue(this->testTime);
    query.addBindValue(ui->operatorIDLineEdit->text());
    //准备update SQL语句
    QString updateStr = "";
    int a[26] = {6,7,8,9,10,10,0,1,2,3,4,5,11,12,21,22,23,24,13,14,16,17,18,19,20,15};//数据库指标顺序对应的项目索引
    QStringList ItemStrList;
    ItemStrList<<gradLean<<leftImageIncline<<rightImageIncline<<relImageIncline<<hAxisParalDegree<<vAxisParalDegree<<leftParal<<rightParal<<
                 leftAdjustRange<<rightAdjustRange<<leftVisZero<<rightVisZero<<leftResolution<<rightResolution<<leftField<<
                 rightField<<leftDistortion<<rightDistortion<<leftExitPupil<<rightExitPupil<<leftExitPupilDistance<<
                 rightExitPupilDistance<<leftRate<<rightRate<<rateDif<<pupilDistanceRange;
    for(int i = 0;i<26;++i){
        if(finishItemFlags[a[i]]){
            updateStr += QString(" `%1`='%2', ").arg(model->headerData(i+2,Qt::Horizontal).toString()).arg(ItemStrList[i]);
        }
    }
    updateStr += QString(" `测试时间`='%1' ").arg(testTime.toString("yyyy-MM-dd HH:mm:ss"));
    if(this->finishItemCount < this->itemCount){
        QString str = "";
        for(int i = 0;i<finishItemFlags.size();++i){
            if(!finishItemFlags[i]){
                str += itemID[i]+" ";
            }
        }
        if(QMessageBox::warning(this,"warning",QString("%1未检测，是否继续保存？").arg(str),QMessageBox::Yes|QMessageBox::No,QMessageBox::No) == QMessageBox::Yes){
            if(query.exec() && query.exec("update result set "+updateStr+QString("where `id`=%1").arg(id)))
            {
                ui->plainTextEdit->appendPlainText("存档成功！");
                ui->tabWidget_2->setCurrentIndex(0);
                clearResult();
                qDebug()<<"[info] 存档成功，更新最新列成功";
            }
            else
            {
                QMessageBox::warning(this,"error","存档失败!!",QMessageBox::Yes);
                ui->plainTextEdit->appendPlainText("存档失败!!");
                ui->tabWidget_2->setCurrentIndex(0);
                qDebug()<<"[error] "<<query.lastError().text()<<" "<<query.lastQuery()<<" "<<query.boundValues();
            }
        }
    }
    if(this->finishItemCount == this->itemCount){
        if(query.exec() && query.exec("update result set "+updateStr+QString("where `id`=%1").arg(id)))
        {
            ui->plainTextEdit->appendPlainText("存档成功！");
            ui->tabWidget_2->setCurrentIndex(0);
            clearResult();
            qDebug()<<"[info] 存档成功，更新最新列成功";
        }
        else
        {
            QMessageBox::warning(this,"error","存档失败!!",QMessageBox::Yes);
            ui->plainTextEdit->appendPlainText("存档失败!!");
            ui->tabWidget_2->setCurrentIndex(0);
            qDebug()<<"[error] "<<query.lastError().text()<<" "<<query.lastQuery();
        }
    }

//    if(this->finishItemCount == this->itemCount)
//    {
//        if(query.exec())
//        {
//            ui->plainTextEdit->appendPlainText("存档成功！");
//            clearResult();
//        }
//        else
//        {
//            QMessageBox::warning(this,"error","存档失败!!",QMessageBox::Yes);
//            ui->plainTextEdit->appendPlainText("存档失败!!");
//            ui->plainTextEdit->appendPlainText(query.lastError().text());
//        }
//    }
//    else
//    {
//        QString str = "";
//        for(int i = 0;i<finishItemFlags.size();++i){
//            if(!finishItemFlags[i]){
//                str += itemID[i]+" ";
//            }
//        }
//        QMessageBox::warning(this,"warning",QString("%1未检测，请继续检测!!").arg(str),QMessageBox::Yes);
//    }

    //状态栏信息恢复
    int tabIndex = ui->tabWidget->currentIndex();
    if(tabIndex == 0)
    {
        statusBar()->showMessage(tr("相机调试..."));
    }
    else if(!ui->productIDLineEdit1->text().isEmpty())
    {
        statusBar()->showMessage(ui->detectItemComboBox->currentText());
    }
    else
        statusBar()->showMessage(tr("指标检测..."));
}

QString MainWindow::degreeUnitTransfer(double value){
    //调用【指标计算】指标的单位精度都是分
    QString resultStr = "";
    if(value < 0){
        resultStr += "-";
        value = abs(value);
    }
    int degreeValue = value/60;
    int minuteValue = value-60*degreeValue;
    int secondValue = 60*(value-60*degreeValue-minuteValue);
    if(degreeValue != 0){
        resultStr += QString::number(degreeValue)+degreeUnitList[0];
    }
    if(minuteValue != 0){
        resultStr += QString::number(minuteValue)+degreeUnitList[1];
    }
    if(secondValue != 0){
        resultStr += QString::number(secondValue)+degreeUnitList[2];
    }
    return resultStr;
}

void MainWindow::printCurrentDetectResult(int defaultIndex)
{
    QString detectItem =  ui->detectItemComboBox->currentText();
    int index = ui->detectItemComboBox->currentIndex();
    if(defaultIndex == 4 || defaultIndex == 5){
         ui->plainTextEdit->appendPlainText(QString("%1 检测结果为%2D 均值为%3D 第%4次 ").arg(ui->detectItemComboBox->itemText(defaultIndex)).
                                            arg(this->ndetectResult).arg(this->aveDetectResult).arg(this->nCurrentCount));
         return ;
     }
    if(index == 10)
    {
        ui->plainTextEdit->appendPlainText(QString("%1 水平检测结果为%2 均值为%3 垂直检测结果为%4 均值为%5 第%6次 (水平上大于0表示发散，小于0表示会聚) ")
                                           .arg(detectItem).arg(degreeUnitTransfer(this->ndetectResult)).arg(degreeUnitTransfer(this->aveDetectResult))
                                           .arg(degreeUnitTransfer(this->ndetectResult_vaxis)).arg(degreeUnitTransfer(this->aveDetectResult_vaxis)).arg(this->nCurrentCount));
    }
    else if(index == 0 || index == 1)
    {
        ui->plainTextEdit->appendPlainText(QString("%1 检测结果为%2 均值为%3 第%6次 ")
                                           .arg(detectItem).arg(degreeUnitTransfer(this->ndetectResult)).arg(degreeUnitTransfer(this->aveDetectResult))
                                           .arg(this->nCurrentCount));
    }
    else if(index == 2 || index == 3){
        ui->plainTextEdit->appendPlainText(QString("%1 检测结果为[%2 %3]D 均值为[%4 %5]D 第%6次 ").arg(detectItem).
                                           arg(this->ndetectResult).arg(this->ndetectResult_vaxis).arg(this->aveDetectResult).arg(this->aveDetectResult_vaxis).arg(this->nCurrentCount));
    }
    else if(index  == 4 || index == 5){
         ui->plainTextEdit->appendPlainText(QString("%1 检测结果为%2D 均值为%3D 第%4次 ").arg(ui->detectItemComboBox->itemText(index)).
                                            arg(this->ndetectResult).arg(this->aveDetectResult).arg(this->nCurrentCount));
     }
    else if(index == 11 || index == 12)
    {
        QString result = QString::number(static_cast<int>(ndetectResult)) + "\u2033" + QString::number(static_cast<int>(ndetectResult_vaxis)).rightJustified(2,'0');
        QString averesult = QString::number(static_cast<int>(aveDetectResult)) + "\u2033" + QString::number(static_cast<int>(aveDetectResult_vaxis)).rightJustified(2,'0');
        ui->plainTextEdit->appendPlainText(QString("%1 检测结果为%2 均值为%3 第%4次 ").arg(detectItem).
                                           arg(result).arg(averesult).arg(this->nCurrentCount));
    }
    else if(index == 21 || index == 22)
    {
        QString result = QString::number(static_cast<int>(ndetectResult)) + "\u00B0" + QString::number(static_cast<int>(ndetectResult_vaxis)) + "\u2032";
        QString averesult = QString::number(static_cast<int>(aveDetectResult)) + "\u00B0" + QString::number(static_cast<int>(aveDetectResult_vaxis)) + "\u2032";
        ui->plainTextEdit->appendPlainText(QString("%1 检测结果为%2 均值为%3 第%4次 ").arg(detectItem).
                                           arg(result).arg(averesult).arg(this->nCurrentCount));
    }
    else if(index == 6 || index == 7 || index == 8 || index == 9){
        ui->plainTextEdit->appendPlainText(QString("%1 检测结果为%2 均值为%3 第%4次 ").arg(detectItem).
                                           arg(degreeUnitTransfer(this->ndetectResult)).arg(degreeUnitTransfer(this->aveDetectResult)).arg(this->nCurrentCount));
    }
    else if(index == 13 || index == 14 || index == 16 || index == 17){
        ui->plainTextEdit->appendPlainText(QString("%1 检测结果为%2mm 均值为%3mm 第%4次 ").arg(detectItem).
                                           arg(this->ndetectResult).arg(this->aveDetectResult).arg(this->nCurrentCount));
    }
    else if(index == 15){
        ui->plainTextEdit->appendPlainText(QString("%1 检测结果为[%2 %3]mm 均值为[%4 %5]mm 第%6次 ").arg(detectItem).
                                           arg(this->ndetectResult).arg(this->ndetectResult_vaxis).arg(this->aveDetectResult).arg(this->aveDetectResult_vaxis).arg(this->nCurrentCount));
    }
    else if(index == 18 || index == 19 || index == 20){
        ui->plainTextEdit->appendPlainText(QString("%1 检测结果为%2X 均值为%3X 第%4次 ").arg(detectItem).
                                           arg(this->ndetectResult).arg(this->aveDetectResult).arg(this->nCurrentCount));
    }
    else if(index == 23 || index == 24){
        ui->plainTextEdit->appendPlainText(QString("%1 检测结果为%2% 均值为%3% 第%4次 ").arg(detectItem).
                                           arg(this->ndetectResult).arg(this->aveDetectResult).arg(this->nCurrentCount));
    }
}



void MainWindow::on_productIDLineEdit1_returnPressed()
{
    QString str = ui->productIDLineEdit1->text();
    //刷新变量
    clearResult();
    this->productID = str;
    ui->productIDLineEdit1->setText(str);   //刷新时刷掉了重新载入
    ui->productIDLineEdit->setText(str);
    //新建保存图片文件夹
    QDir dir;
    QString dirPath = QString("./assets/%1").arg(this->productID);
    if(!dir.mkpath(dirPath))
    {
        QMessageBox::warning(this,"error","创建文件夹失败");
    }
//    //查询数据库中是否存在该产品的检测数据
//    QSqlQuery query;
//    query.exec(QString("select * from result where `产品编号` = %1 order by `id` asc").arg(productID));
//    if(query.next())
//    {
//        qDebug()<<"[info]数据库中存在该产品的数据";
//        this->productID = query.value(1).toString();
//        ui->productIDLineEdit->setText(this->productID);
//        this->leftParal = query.value(7).toDouble();
//        ui->leftParalLineEdit->setText(QString::number(this->leftParal));
//        this->rightParal = query.value(8).toDouble();
//        ui->rightParalLineEdit->setText(QString::number(this->rightParal));
//        this->leftAdjustRange = query.value(9).toString();
//        ui->leftAdjustRangeLineEdit->setText(this->leftAdjustRange);
//        this->rightAdjustRange = query.value(10).toString();
//        ui->rightAdjustRangeLineEdit->setText(this->rightAdjustRange);
//        this->leftVisZero = query.value(11).toDouble();
//        ui->leftVisZeroLineEdit->setText(QString::number(this->leftVisZero));
//        this->rightVisZero = query.value(12).toDouble();
//        ui->rightVisZeroLineEdit->setText(QString::number(this->rightVisZero));
//        this->gradLean = query.value(2).toDouble();
//        ui->gradLeanLineEdit->setText(QString::number(this->gradLean));
//        this->imageIncline = query.value(3).toDouble();
//        ui->imageInclineLineEdit->setText(QString::number(this->imageIncline));
//        this->relImageIncline = query.value(4).toDouble();
//        ui->relImageInclineLineEdit->setText(QString::number(this->relImageIncline));
//        this->hAxisParalDegree = query.value(5).toDouble();
//        ui->hAxisParalDegreeLineEdit->setText(QString::number(this->hAxisParalDegree));
//        this->vAxisParalDegree = query.value(6).toDouble();
//        ui->vAxisParalDegreeLineEdit->setText(QString::number(this->vAxisParalDegree));
//        this->leftResolution = query.value(13).toString();
//        ui->leftResolutionLneEdit->setText(this->leftResolution);
//        this->rightResolution = query.value(14).toString();
//        ui->rightResolutionLineEdit->setText(this->rightResolution);
//        this->leftExitPupil = query.value(19).toDouble();
//        ui->leftExitPupilLineEdit->setText(QString::number(this->leftExitPupil));
//        this->rightExitPupil = query.value(20).toDouble();
//        ui->rightExitPupilLineEdit->setText(QString::number(this->rightExitPupil));
//        this->pupilDistanceRange = query.value(26).toDouble();
//        ui->pupilDistanceRangeLineEdit->setText(QString::number(this->pupilDistanceRange));
//        this->leftExitPupilDistance = query.value(21).toDouble();
//        ui->leftExitPupilDistanceLineEdit->setText(QString::number(this->leftExitPupilDistance));
//        this->rightExitPupilDistance = query.value(22).toDouble();
//        ui->rightExitPupilDistanceLineEdit->setText(QString::number(this->rightExitPupilDistance));
//        this->leftRate = query.value(23).toDouble();
//        ui->leftRateLineEdit->setText(QString::number(this->leftRate));
//        this->rightRate = query.value(24).toDouble();
//        ui->rightRateLineEdit->setText(QString::number(this->rightRate));
//        this->rateDif = query.value(25).toDouble();
//        ui->rateDifLineEdit->setText(QString::number(this->rateDif));
//        this->leftField = query.value(15).toString();
//        ui->leftFieldLineEdit->setText(this->leftField);
//        this->rightField = query.value(16).toString();
//        ui->rightFieldLineEdit->setText(this->rightField);
//        this->leftDistortion = query.value(17).toDouble();
//        ui->leftDistortionLineEdit->setText(QString::number(this->leftDistortion));
//        this->rightDistortion = query.value(18).toDouble();
//        ui->rightDistortionLineEdit->setText(QString::number(this->rightDistortion));
//    }
//    else
//        qDebug()<<"[info]数据库中不存在该产品的数据";
    //状态栏更新
    if(str.isEmpty())
    {
        statusBar()->showMessage(tr("指标检测..."));
    }
    else
        statusBar()->showMessage(ui->detectItemComboBox->currentText());
}

//清理检测结果 除了操作员ID
void MainWindow::clearResult()
{
    //指标结果清零
    ui->gradLeanLineEdit->clear();
    ui->leftImageInclineLineEdit->clear();
    ui->rightImageInclineLineEdit->clear();
    ui->relImageInclineLineEdit->clear();
    ui->hAxisParalDegreeLineEdit->clear();
    ui->vAxisParalDegreeLineEdit->clear();
    ui->leftParalLineEdit->clear();
    ui->rightParalLineEdit->clear();
    ui->leftAdjustRangeLineEdit->clear();
    ui->rightAdjustRangeLineEdit->clear();
    ui->leftVisZeroLineEdit->clear();
    ui->rightVisZeroLineEdit->clear();
    ui->leftResolutionLneEdit->clear();
    ui->rightResolutionLineEdit->clear();
    ui->leftFieldLineEdit->clear();
    ui->rightFieldLineEdit->clear();
    ui->leftDistortionLineEdit->clear();
    ui->rightDistortionLineEdit->clear();
    ui->leftExitPupilLineEdit->clear();
    ui->rightExitPupilLineEdit->clear();
    ui->leftExitPupilDistanceLineEdit->clear();
    ui->rightExitPupilDistanceLineEdit->clear();
    ui->leftRateLineEdit->clear();
    ui->rightRateLineEdit->clear();
    ui->rateDifLineEdit->clear();
    ui->pupilDistanceRangeLineEdit->clear();
    ui->productIDLineEdit->clear();
    ui->testTimeLineEdit->clear();
    //清理产品编号输入
    ui->productIDLineEdit1->clear();
    //清空单元号 检测结果
    ui->unitLineEdit_4->clear();
    ui->resultLineEdit_4->clear();
    ui->ruleDistanceLineEdit->clear();
    ui->sharpnessLineEdit_3->clear();
    //过程变量清零
    this->ndetectResult = 0;
    this->aveDetectResult = 0;
    this->ndetectResult_vaxis = 0;
    this->aveDetectResult_vaxis = 0;
    this->nCurrentCount = 0;
    this->finishItemCount = 0;
    this->currentItemIndex = 0;  
    //重置已完成项目的flags
    this->finishItemFlags.clear();
    this->finishItemFlags = QVector<bool>(this->itemCount,false);
    this->leftImageInclineArray.clear();
    this->rightImageInclineArray.clear();
    this->leftRateArray.clear();
    this->rightRateArray.clear();
    this->leftVisZeroArray.clear();
    this->rightVisZeroArray.clear();
    //重置指标区
    ui->detectItemComboBox->setCurrentIndex(0);
    ui->productComboBox->setCurrentIndex(0);
    itemLayout(0);
    //使能
    ui->startGrabButton->setEnabled(true);
    ui->getImageButton->setEnabled(false);
    ui->imageProcessButton->setEnabled(false);
    ui->indicatorComputeButton->setEnabled(false);




}

void MainWindow::setLineEditReadOnly(bool isReadOnly)
{
    ui->gradLeanLineEdit->setReadOnly(isReadOnly);
    ui->leftImageInclineLineEdit->setReadOnly(isReadOnly);
    ui->rightImageInclineLineEdit->setReadOnly(isReadOnly);
    ui->relImageInclineLineEdit->setReadOnly(isReadOnly);
    ui->hAxisParalDegreeLineEdit->setReadOnly(isReadOnly);
    ui->vAxisParalDegreeLineEdit->setReadOnly(isReadOnly);
    ui->leftParalLineEdit->setReadOnly(isReadOnly);
    ui->rightParalLineEdit->setReadOnly(isReadOnly);
    ui->leftAdjustRangeLineEdit->setReadOnly(isReadOnly);
    ui->rightAdjustRangeLineEdit->setReadOnly(isReadOnly);
    ui->leftVisZeroLineEdit->setReadOnly(isReadOnly);
    ui->rightVisZeroLineEdit->setReadOnly(isReadOnly);
    ui->leftResolutionLneEdit->setReadOnly(isReadOnly);
    ui->rightResolutionLineEdit->setReadOnly(isReadOnly);
    ui->leftFieldLineEdit->setReadOnly(isReadOnly);
    ui->rightFieldLineEdit->setReadOnly(isReadOnly);
    ui->leftDistortionLineEdit->setReadOnly(isReadOnly);
    ui->rightDistortionLineEdit->setReadOnly(isReadOnly);
    ui->leftExitPupilLineEdit->setReadOnly(isReadOnly);
    ui->rightExitPupilLineEdit->setReadOnly(isReadOnly);
    ui->leftExitPupilDistanceLineEdit->setReadOnly(isReadOnly);
    ui->rightExitPupilDistanceLineEdit->setReadOnly(isReadOnly);
    ui->leftRateLineEdit->setReadOnly(isReadOnly);
    ui->rightRateLineEdit->setReadOnly(isReadOnly);
    ui->rateDifLineEdit->setReadOnly(isReadOnly);
    ui->pupilDistanceRangeLineEdit->setReadOnly(isReadOnly);
    ui->productIDLineEdit->setReadOnly(isReadOnly);
    ui->testTimeLineEdit->setReadOnly(isReadOnly);
    ui->operatorIDLineEdit->setReadOnly(isReadOnly);
    //设置相机调试只读
    ui->FrameRateLineEdit->setReadOnly(isReadOnly);
    //设置sharpness只读
    ui->sharpnessLineEdit->setReadOnly(isReadOnly);
    ui->sharpnessLineEdit_3->setReadOnly(isReadOnly);
    //设置离焦距离值
    ui->ruleDistanceLineEdit->setReadOnly(isReadOnly);
}

void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    if(index ==0)
    {
        statusBar()->showMessage(tr("相机调试..."));
    }
    else if(index == 1)
    {
        if(ui->productIDLineEdit1->text().isEmpty())
        {
            statusBar()->showMessage(tr("指标检测..."));
        }
        else
        {
            statusBar()->showMessage(ui->detectItemComboBox->currentText());
        }
    }
}

void MainWindow::cameraParaInit(float exposeTime,float gain,float frameRate,bool autoExposeTime,bool autoGain){
    //先关掉自动曝光和自动增益后设置相机参数
    on_checkBox_2_clicked(autoExposeTime);
    on_checkBox_3_clicked(autoGain);
    this->m_mvCamera->SetExposureTime(exposeTime);
    this->m_mvCamera->SetGain(gain);
    this->m_mvCamera->SetAcquisitionFrameRate(frameRate);

    on_GetParamButton_clicked();
}

void MainWindow::on_detectItemComboBox_activated(int index)
{
    if(ui->productIDLineEdit1->text().isEmpty())
    {
        QMessageBox::warning(this,"提示","请输入产品编号",QMessageBox::Yes);
        ui->detectItemComboBox->setCurrentIndex(this->currentItemIndex);
        return ;
    }
    if(nCurrentCount != 0)
    {
        if(QMessageBox::warning(this,"提示","确定放弃当前测量的结果吗？",QMessageBox::Yes|QMessageBox::No,QMessageBox::No) == QMessageBox::Yes)
        {
            this->nCurrentCount = 0;
        }
        else
        {
            ui->detectItemComboBox->setCurrentIndex(this->currentItemIndex);
            return ;
        }
    }
    if(this->finishItemFlags[index] && index != 4 && index != 5)
    {
        if(QMessageBox::warning(this,"提示","该项目已经检测，是否重新检测？",QMessageBox::Yes|QMessageBox::No,QMessageBox::No) == QMessageBox::No)
        {
            ui->detectItemComboBox->setCurrentIndex(this->currentItemIndex);
            return;
        }
    }
    if(index == 9 ){
        QString str = "";
        if(!finishItemFlags[7]){
            str += "左像倾斜、";
        }
        if(!finishItemFlags[8]){
            str += "右像倾斜";
        }
        if(!str.isEmpty()){
            QMessageBox::warning(this,"提示",QString("请完成%1检测！").arg(str),QMessageBox::Yes);
            ui->detectItemComboBox->setCurrentIndex(this->currentItemIndex);
            return;
        }
    }
    if(index == 20){
        QString str = "";
        if(!finishItemFlags[18]){
            str += "左倍率、";
        }
        if(!finishItemFlags[19]){
            str += "右倍率";
        }
        if(!str.isEmpty()){
            QMessageBox::warning(this,"提示",QString("请完成%1检测！").arg(str),QMessageBox::Yes);
            ui->detectItemComboBox->setCurrentIndex(this->currentItemIndex);
            return;
        }
    }
    if(!ui->productIDLineEdit1->text().isEmpty())
    {
        QString str = ui->detectItemComboBox->currentText();
        statusBar()->showMessage(str);
    }
    if((index == 4 && finishItemFlags[0]) || (index ==5 && finishItemFlags[1]))
    {
        if(QMessageBox::warning(this,"提示","检测视差时已经检测视度零位，是否重复检测",QMessageBox::Yes|QMessageBox::No,QMessageBox::No) == QMessageBox::No)
        {
            ui->detectItemComboBox->setCurrentIndex(this->currentItemIndex);
//            if(index == 4)
//                ui->leftVisZeroLineEdit->setText(this->leftVisZero);
//            if(index == 5)
//                ui->rightVisZeroLineEdit->setText(this->rightVisZero);
//            finishItemFlags[index] = true;
//            finishItemCount++;
            if(index == 4){
                for(int i = 0;i<6;++i){
                    this->ndetectResult = leftVisZeroArray[i];
                    this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1);
                    nCurrentCount++;
                    printCurrentDetectResult(4);
                }
            }
            if(index == 5){
                for(int i = 0;i<6;++i){
                    this->ndetectResult = rightVisZeroArray[i];
                    this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1);
                    nCurrentCount++;
                    printCurrentDetectResult(5);
                }
            }
            nCurrentCount = 0;
            ui->tabWidget_2->setCurrentIndex(0);
            return;
        }
    }
    //使能
    ui->startGrabButton->setEnabled(true);
    ui->getImageButton->setEnabled(false);
    ui->imageProcessButton->setEnabled(false);
    ui->indicatorComputeButton->setEnabled(false);
    ui->leftLimitButton->setEnabled(false);
    ui->rightLimitButton->setEnabled(false);
    //使能离焦距离值groups
    ui->groupBox_4->setEnabled(false);
    //布局
    itemLayout(index);
    //相机参数初始化
    if(ui->ExposureLineEdit->isEnabled()){
        if(index<=12 && index != 9 && index != 7){
            on_checkBox_2_clicked(true);
            on_checkBox_3_clicked(true);
        }
        if(index == 7){
            cameraParaInit();
        }
    }
    this->currentItemIndex = index;
    //进度条+1  step=1
    myProgressBarStepUpdate(1);
    if(index == 2 ||index ==3){
        this->isDetectLeftLimit = false;
    }

}

void MainWindow::setScrollBarWHOffset(int offsetX,int offsetY){
    ui->scrollArea->horizontalScrollBar()->setValue(offsetX);
    ui->scrollArea->verticalScrollBar()->setValue(offsetY);
    qDebug()<<offsetX<<" "<<ui->scrollArea->horizontalScrollBar()->value();
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    if(checked)
    {
        ui->displayLabel->setFixedSize(ui->displayLabel->pixmap()->width(),ui->displayLabel->pixmap()->height());
        this->isChecked = true;
    }
    else
    {
        ui->displayLabel->setFixedSize(ui->scrollArea->width(),ui->scrollArea->height());
        ui->displayLabel->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    }
}
//程序启动窗口是隐藏状态，所以控件的大小都是默认大小，一般show之间会调用resizeevent 但是那时候的大小还是默认，所以最好在show以后获取控件的大小
void MainWindow::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    ui->displayLabel->setFixedSize(ui->scrollArea->width(),ui->scrollArea->height());
    this->toolbar->setFixedWidth(ui->displayGroup->width()+ui->groupBox_13->width()-timeLabel->width());
    qDebug()<<"[info] showevent "<<ui->displayLabel->size();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    ui->displayLabel->setFixedSize(ui->scrollArea->width(),ui->scrollArea->height());
    ui->checkBox->setChecked(false);
    //固定状态栏中的工具栏的width
    this->toolbar->setFixedWidth(ui->displayGroup->width()+ui->groupBox_13->width()-timeLabel->width());

    qDebug()<<"[info] reseizeevent "<<ui->displayLabel->size();
    qDebug()<<ui->displayGroup->width()<<" "<<ui->groupBox_13->width()<<" "<<timeLabel->width()<<" = "<<this->toolbar->width();
}

void MainWindow::splitterMove(int pos,int index)
{
    Q_UNUSED(pos);
    Q_UNUSED(index);
    ui->displayLabel->setFixedSize(ui->scrollArea->width(),ui->scrollArea->height());
    ui->checkBox->setChecked(false);
}

//设置自动曝光
void MainWindow::on_checkBox_2_clicked(bool checked)
{
    if(checked)
    {
        int ret = this->m_mvCamera->SetExposureTimeAuto();
        if(ret == MV_OK)
        {
            this->isAutoExposureTime = true;
            on_GetParamButton_clicked();
            ui->checkBox_2->setChecked(checked);
            ui->checkBox_4->setChecked(checked);
        }
        else
        {
            ui->checkBox_2->setChecked(false);
            ui->checkBox_4->setChecked(false);
            QMessageBox::critical(this,"error","设置自动曝光失败");
        }
    }
    else
    {
        int ret = this->m_mvCamera->CloseExposureTimeAuto();
        if(ret == MV_OK)
        {
            this->isAutoExposureTime = false;
            on_GetParamButton_clicked();
            ui->checkBox_2->setChecked(checked);
            ui->checkBox_4->setChecked(checked);
        }
        else
        {
            ui->checkBox_2->setChecked(true);
            ui->checkBox_4->setChecked(true);
            QMessageBox::critical(this,"error","关闭自动曝光失败");
        }
    }


}

//设置自动增益
void MainWindow::on_checkBox_3_clicked(bool checked)
{
    if(checked)
    {
        int ret = this->m_mvCamera->SetGainAuto();
        if(ret == MV_OK)
        {
            this->isAutoGain = true;
            on_GetParamButton_clicked();
            ui->checkBox_3->setChecked(checked);
            ui->checkBox_5->setChecked(checked);
        }
        else
        {
            ui->checkBox_3->setChecked(false);
            ui->checkBox_5->setChecked(false);
            QMessageBox::critical(this,"error","设置自动增益失败");
        }
    }
    else
    {
        int ret = this->m_mvCamera->CloseGainAuto();
        if(ret == MV_OK)
        {
            this->isAutoGain = false;
            on_GetParamButton_clicked();
            ui->checkBox_3->setChecked(checked);
            ui->checkBox_5->setChecked(checked);
        }
        else
        {
            ui->checkBox_3->setChecked(true);
            ui->checkBox_5->setChecked(true);
            QMessageBox::critical(this,"error","关闭自动增益失败");
        }
    }
}


void MainWindow::on_startGrabButton_clicked()
{
    if(ui->productIDLineEdit1->text().isEmpty())
    {
        QMessageBox::warning(this,"提示","请输入产品编号",QMessageBox::Yes);
        return ;
    }
    if(m_getImageThread->m_ThreadEnabled)
    {
        //使能
        ui->startGrabButton->setEnabled(false);
        ui->getImageButton->setEnabled(true);
        ui->groupBox_10->setEnabled(true);
        //关闭 相机调试采集功能
        ui->startGetImageButton->setEnabled(false);
        ui->stopGetImageButton->setEnabled(false);
        ui->imageProcessButton->setEnabled(false);
        ui->indicatorComputeButton->setEnabled(false);
        ui->leftLimitButton->setEnabled(false);
        ui->rightLimitButton->setEnabled(false);
        //使能离焦距离值groups
        ui->groupBox_4->setEnabled(true);
        return;
    }

    // 调用相机库，开始流抓取
    int nRet = m_mvCamera->StartGrabbing();

    if(nRet == MV_OK)
    {
        // 创建 设置 启动 图片抓取线程
        m_getImageThread->setGetImageThreadFlag(true);
        m_getImageThread->start();
        //使能
        ui->startGrabButton->setEnabled(false);
        ui->getImageButton->setEnabled(true);
        ui->imageProcessButton->setEnabled(false);
        ui->indicatorComputeButton->setEnabled(false);
        ui->leftLimitButton->setEnabled(false);
        ui->rightLimitButton->setEnabled(false);
        ui->groupBox_10->setEnabled(true);
        //相机调试使能
        ui->startGetImageButton->setEnabled(false);
        ui->stopGetImageButton->setEnabled(false);
        //使能离焦距离值groups
        ui->groupBox_4->setEnabled(true);
    }
    else
    {
        QMessageBox::warning(this, "异常", "图像抓取失败！");
        return;
    }
    //进度条步骤更新 防止没有点击项目下拉框或没有参数设置
    if(this->isMatchRightPoint) return;
    if(currentItemIndex == 2 || currentItemIndex == 3){
        if(this->isDetectLeftLimit){
            myProgressBarStepUpdate(2);
        }
        else
        {
            myProgressBarStepUpdate(1);
        }
    }
    else if(currentItemIndex != 0 && currentItemIndex != 1 && currentItemIndex != 4 && currentItemIndex != 5)
    {
        //进度条+1  step=1
        myProgressBarStepUpdate(1);
    }
    else
    {
        //进度条+1  step=2
        myProgressBarStepUpdate(2);
    }
}

void MainWindow::on_getImageButton_clicked()
{
    if(ui->productIDLineEdit1->text().isEmpty())
    {
        QMessageBox::warning(this,"提示","请输入产品编号",QMessageBox::Yes);
        return ;
    }
    // 关闭 图片抓取线程
    m_getImageThread->setGetImageThreadFlag(false);
    m_getImageThread->wait();  // 等待线程结束; wait()函数在 5.15 中引入
    m_getImageThread->exit(0); // 等价于 quit()

    // 调用相机库，取消流抓取
    int nRet = m_mvCamera->StopGrabbing();
    if(nRet != MV_OK)
    {
        QMessageBox::information(this, "异常", "停止抓取失败！");
        return;
    }
    //使能
    ui->getImageButton->setEnabled(false);
    ui->startGrabButton->setEnabled(true);     //打开开始采集，实现放弃本次检测功能
    ui->groupBox_10->setEnabled(false);
    //使能光栅尺
    ui->groupBox_4->setEnabled(false);
    //关闭鼠标点击和框选检测区域的功能
    ui->displayLabel->isOnlyClick = false;
    ui->displayLabel->isNeedRect = false;
    int index = ui->detectItemComboBox->currentIndex();
    if(index == 6 || index == 7 || index == 8 || index == 10 )
    {
        ui->imageProcessButton->setEnabled(true);
    }
    else if(index == 0 || index == 1 || index == 4 || index == 5)
    {
        ui->indicatorComputeButton->setEnabled(true);
    }
    else if(index == 2 || index == 3){
        if(!this->isDetectLeftLimit){
            ui->leftLimitButton->setEnabled(true);
        }
        else {
            ui->rightLimitButton->setEnabled(true);
        }
    }
    //进度条更新
    if(this->isMatchRightPoint) return;
    if(currentItemIndex == 2 || currentItemIndex == 3){
        return ;
    }
    else if(currentItemIndex != 0 && currentItemIndex != 1 && currentItemIndex != 4 && currentItemIndex != 5)
    {
        //进度条+1  step=2
        myProgressBarStepUpdate(2);
    }
    else
    {
        //进度条+1  step=3
        myProgressBarStepUpdate(3);
    }
}

void MainWindow::on_imageProcessButton_clicked()
{
    //使能
    ui->imageProcessButton->setEnabled(false);
    int index = ui->detectItemComboBox->currentIndex();
    //检测分划倾斜、像倾斜、相对像倾斜
    if(index == 6 || index == 7 || index == 8 )
    {
        //直线检测
        if(!detect_lines()) return;
//        QMessageBox::information(this,"提示","请点击要检测的直线");
        //设置displaylabel只能够点击
        ui->displayLabel->isOnlyClick = true;
    }
    //检测光轴平行度
    else if(index == 10)
    {
//        QMessageBox::information(this,"提示","请框选检测平行光管十字分划中心区域(面积尽可能的大)",QMessageBox::Yes);
//        ui->displayLabel->isNeedRect = true;
//        QMessageBox::information(this,"提示","请点击十字分划中心");
        ui->displayLabel->isOnlyClick = true;
    }

}

void MainWindow::on_indicatorComputeButton_clicked()
{
    ui->tabWidget_2->setCurrentIndex(0);
    //使能
    ui->indicatorComputeButton->setEnabled(false);
    int index = ui->detectItemComboBox->currentIndex();
    if(index == 10 && !this->isMatchRightPoint){
        this->leftOriginCenterPos = originCenterPos;
        this->isMatchRightPoint = true;
        ui->displayLabel->isOnlyClick = false;
        myProgressBarStepUpdate(3);
        return;
    }
    if(index == 6 || index == 7 || index == 8)
    {
        if(index == 7) leftImageInclineArray[nCurrentCount] = ndetectResult;
        if(index == 8) rightImageInclineArray[nCurrentCount] = ndetectResult;
        this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1) ;
        nCurrentCount++;
        //检测结束后 恢复变量
        ui->displayLabel->isOnlyClick = false;
    }
    else if(index == 10 && isMatchRightPoint)
    {
        this->ndetectResult = 60*atan((leftOriginCenterPos.x() - originCenterPos.x())*1.85/1000/this->lenFocus/this->defaultMagnification)*180/CV_PI;
        this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1) ;
        this->ndetectResult_vaxis = 60*atan(abs(leftOriginCenterPos.y() - originCenterPos.y())*1.85/1000/this->lenFocus/this->defaultMagnification)*180/CV_PI;
        this->aveDetectResult_vaxis = (aveDetectResult_vaxis * nCurrentCount + ndetectResult_vaxis)/(double)(nCurrentCount +1); 
        nCurrentCount++;
        this->isMatchRightPoint = false;
        ui->displayLabel->isOnlyClick = false;
        ui->plainTextEdit->appendPlainText(QString::number(leftOriginCenterPos.x())+" "+QString::number(leftOriginCenterPos.y()));
        ui->plainTextEdit->appendPlainText(QString::number(originCenterPos.x())+" "+QString::number(originCenterPos.y()));
    }
    else if(index == 0 ||index == 1)
    {
        if(!lenFocus || !pP2 || !pExitPupilDistance || !pMagnification)
        {
            QMessageBox::warning(this,"提示","出瞳直径、放大率、镜头焦距、P2不能为0");
            return;
        }
        //计算视度零位
        this->ndetectResult_vaxis = 1000*defocusDistance/(-lenFocus*lenFocus-(lenFocus-pP2)*defocusDistance);
        this->aveDetectResult_vaxis = (aveDetectResult_vaxis * nCurrentCount + ndetectResult_vaxis)/(double)(nCurrentCount +1);
        if(index == 0) this->leftVisZeroArray[nCurrentCount] = ndetectResult_vaxis;
        if(index == 1) this->rightVisZeroArray[nCurrentCount] = ndetectResult_vaxis;
        //计算视差
        this->ndetectResult = 3.438*pExitPupilDistance*ndetectResult_vaxis/pMagnification;
        this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1);
        nCurrentCount++;
        qDebug()<<ndetectResult<<" "<<aveDetectResult;
    }
    else if(index == 4 || index == 5)
    {
        if(!lenFocus || !pP2)
        {
            QMessageBox::warning(this,"提示","镜头焦距、P2不能为0");
            return;
        }
        this->ndetectResult = 1000*defocusDistance/(-lenFocus*lenFocus-(lenFocus-pP2)*defocusDistance);
        this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1);
        nCurrentCount++;
    }
    if(index != 9 || index != 20){
        printCurrentDetectResult();
    }
    //计算相对像倾斜 左像倾斜-右像倾斜
    if(index == 9){
        for(int i = 0;i<6;++i){
            this->ndetectResult = leftImageInclineArray[i] - rightImageInclineArray[i];
            this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1);
            nCurrentCount++;
            printCurrentDetectResult();
        }
    }
    //计算倍率差
    if(index == 20){
        for(int i = 0;i<6;++i){
            this->ndetectResult = leftRateArray[i] - rightRateArray[i];
            this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1);
            nCurrentCount++;
            printCurrentDetectResult();
        }
    }
    //检测次数达到目标总次数
    if(nCurrentCount == this->testCount)
    {      
        if(index == 6)
        {
            this->gradLean = degreeUnitTransfer(this->aveDetectResult);
            ui->gradLeanLineEdit->setText(this->gradLean);
        }
        else if(index == 7)
        {
            this->leftImageIncline = degreeUnitTransfer(this->aveDetectResult);
            ui->leftImageInclineLineEdit->setText(this->leftImageIncline);
        }
        else if(index == 8)
        {
            this->rightImageIncline = degreeUnitTransfer(this->aveDetectResult);
            ui->rightImageInclineLineEdit->setText(this->rightImageIncline);
        }
        else if(index == 9)
        {
            this->relImageIncline = degreeUnitTransfer(this->aveDetectResult);
            ui->relImageInclineLineEdit->setText(this->relImageIncline);
        }
        else if(index == 20)
        {
            this->rateDif = QString::number(this->aveDetectResult)+"X";
            ui->rateDifLineEdit->setText(this->rateDif);
        }
        else if(index == 10)
        {
            this->hAxisParalDegree = degreeUnitTransfer(this->aveDetectResult);
            this->vAxisParalDegree = degreeUnitTransfer(this->aveDetectResult_vaxis);
            ui->hAxisParalDegreeLineEdit->setText(this->hAxisParalDegree);
            ui->vAxisParalDegreeLineEdit->setText(this->vAxisParalDegree);
        }
        else if(index == 0)
        {
            this->leftVisZero = degreeUnitTransfer(this->aveDetectResult_vaxis);
            this->leftParal = degreeUnitTransfer(this->aveDetectResult);
            ui->leftParalLineEdit->setText(this->leftParal);
            ui->leftVisZeroLineEdit->setText(this->leftVisZero);
            if(!finishItemFlags[4]){
                finishItemFlags[4] = true;
                finishItemCount++;
            }
        }
        else if(index == 1)
        {
            this->rightVisZero = degreeUnitTransfer(this->aveDetectResult_vaxis);
            this->rightParal = degreeUnitTransfer(this->aveDetectResult);
            ui->rightParalLineEdit->setText(this->rightParal);
            ui->rightVisZeroLineEdit->setText(this->rightVisZero);
            if(!finishItemFlags[5]){
                finishItemFlags[5] = true;
                finishItemCount++;
            }
        }
        else if(index == 4)
        {
            this->leftVisZero = QString::number(this->aveDetectResult);
            ui->leftVisZeroLineEdit->setText(this->leftVisZero);
        }
        else if(index == 5)
        {
            this->rightVisZero = QString::number(this->aveDetectResult)+"D";
            ui->rightVisZeroLineEdit->setText(this->rightVisZero);
        }
        //保存图片
        QImage img((uchar*)image_BGR_display.data,image_BGR_display.cols,image_BGR_display.rows,image_BGR_display.cols * 3,QImage::Format_RGB888);
        QDir dir;
        QString dirPath = QString("./assets/%1").arg(this->productID);
        if(dir.mkpath(dirPath))
        {   //图片格式是png会出错，估计是压缩了
            QString save_path = QDir(dirPath).path()+"/"+QString("%2.%3").arg(this->itemID[this->currentItemIndex]).arg(this->saveFormat);
            img.save(save_path,const_cast<const char*>((char*)this->saveFormat.toUtf8().data()),100);
        }
        nCurrentCount = 0;
        if(!this->finishItemFlags[index])
        {
            this->finishItemFlags[index] = true;
            finishItemCount++;
        }
        //跳转之resultTab
        ui->tabWidget_2->setCurrentIndex(1);
    }
    //进度条更新
    if(currentItemIndex != 0 && currentItemIndex != 1 && currentItemIndex != 4 && currentItemIndex != 5 && currentItemIndex != 10)
    {
        //进度条+1  step=3
        myProgressBarStepUpdate(3);
    }
    else
    {
        //进度条+1  step=4
        myProgressBarStepUpdate(4);
    }
}

void MainWindow::on_leftLimitButton_clicked()
{
    //使能
    ui->leftLimitButton->setEnabled(false);
    int index = ui->detectItemComboBox->currentIndex();
    if(index == 2 || index == 3)
    {
        if(!lenFocus || !pP2)
        {
            QMessageBox::warning(this,"提示","镜头焦距、P2不能为0");
            return;
        }
        this->ndetectResult = 1000*defocusDistance/(-lenFocus*lenFocus-(lenFocus-pP2)*defocusDistance)+4;
        this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1);
        this->isDetectLeftLimit = true;
    }
    myProgressBarStepUpdate(2);
}

void MainWindow::on_rightLimitButton_clicked()
{
    //使能
    ui->rightLimitButton->setEnabled(false);
    ui->tabWidget_2->setCurrentIndex(0);
    int index = ui->detectItemComboBox->currentIndex();
    if(index == 2 || index == 3)
    {
        if(!lenFocus || !pP2)
        {
            QMessageBox::warning(this,"提示","镜头焦距、P2不能为0");
            return;
        }
        this->ndetectResult_vaxis = 1000*defocusDistance/(-lenFocus*lenFocus-(lenFocus-pP2)*defocusDistance)+4;
        this->aveDetectResult_vaxis = (aveDetectResult_vaxis * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1);
        nCurrentCount++;
    }
    printCurrentDetectResult();
    //检测次数达到目标总次数
    if(nCurrentCount == this->testCount)
    {
        if(index == 2)
        {
            this->leftAdjustRange ="[" + QString::number(aveDetectResult) + " " + QString::number(aveDetectResult_vaxis)+"]D";
            ui->leftAdjustRangeLineEdit->setText(leftAdjustRange);
        }
        else if(index == 3)
        {
            this->rightAdjustRange ="[" + QString::number(aveDetectResult) + " " + QString::number(aveDetectResult_vaxis)+"]D";
            ui->rightAdjustRangeLineEdit->setText(rightAdjustRange);
        }
        //保存图片
        QImage img((uchar*)image_BGR_display.data,image_BGR_display.cols,image_BGR_display.rows,image_BGR_display.cols * 3,QImage::Format_RGB888);
        QDir dir;
        QString dirPath = QString("./assets/%1").arg(this->productID);
        if(dir.mkpath(dirPath))
        {   //图片格式是png会出错，估计是压缩了
            QString save_path = QDir(dirPath).path()+"/"+QString("%2.%3").arg(this->itemID[this->currentItemIndex]).arg(this->saveFormat);
            img.save(save_path,const_cast<const char*>((char*)this->saveFormat.toUtf8().data()),100);
        }
        nCurrentCount = 0;
        if(!this->finishItemFlags[index])
        {
            this->finishItemFlags[index] = true;
            finishItemCount++;
        }
        //跳转之resultTab
        ui->tabWidget_2->setCurrentIndex(1);
    }
    //进度条更新
    //进度条+1  step=4
    myProgressBarStepUpdate(3);
    //状态恢复
    this->isDetectLeftLimit = false;
}

void MainWindow::matchLine(QPoint point)
{
    //光轴平行度点击计算分划中心
    if(this->currentItemIndex == 10){
        this->originCenterPos = point;
        cv::Mat image_BGR_clone = image_BGR_display.clone();
        cv::circle(image_BGR_clone,cv::Point(originCenterPos.x(),originCenterPos.y()),4,cv::Scalar(0,0,255),-1);
        ui->displayLabel->setPixmap(QPixmap::fromImage(p->cvMat2QImage(image_BGR_clone)));
        qDebug()<<"circle";
        //使能 指标计算
        ui->indicatorComputeButton->setEnabled(true);
        return ;
    }
    int matchIndex = 0;
    float mindist = DBL_MAX;
    for(unsigned int i = 0; i < detecteLines.size(); i++ )
    {
        cv::Vec4i line = detecteLines[i];
        double a = line[3] - line[1];
        double b = line[0] - line[2];
        double c = line[2]*line[1] - line[0]*line[3];
        double dist = abs(a*point.x()+b*point.y()+c)/sqrt(a*a + b*b);
        if(dist < mindist)
        {
            mindist  = dist;
            matchIndex = i;
        }
    }
    double angle = 0 ;
    angle = atan((double)(detecteLines[matchIndex][3]-detecteLines[matchIndex][1])/(double)(detecteLines[matchIndex][0]-detecteLines[matchIndex][2])) * 180 / CV_PI;
    //结果
    ndetectResult = (90-abs(angle))*60;
    qDebug()<<"[matchline] mattchindex :"<< matchIndex <<" angle "<<angle<<" point "
            <<detecteLines[matchIndex][0]<<" "<<detecteLines[matchIndex][1]
            <<" "<<detecteLines[matchIndex][2]<<" "<<detecteLines[matchIndex][3];

    //使能 指标计算
    ui->indicatorComputeButton->setEnabled(true);

}

void MainWindow::matchPoint(cv::Rect rect)
{
    if(detect_point(rect))
    {
        //使能 指标计算
        ui->indicatorComputeButton->setEnabled(true);
        //检测结束后 恢复变量
        ui->displayLabel->isNeedRect = false;
    }
}

bool MainWindow::detect_lines()
{
    try
    {
        cv::Mat image_gray = this->image_GRAY.clone();
        p->gaussianBlur(image_gray);
        p->enhanceContrast(image_gray);
        if(currentItemIndex == 7 || currentItemIndex == 8){
            p->binarization(image_gray,180);
        }
        else
            p->binarization(image_gray,100);
        cv::namedWindow("image",cv::WINDOW_FREERATIO);
        cv::imshow("image",image_gray);
        p->canny(image_gray);
        p->dilateErode(image_gray);
        this->detecteLines = p->hough(image_gray,50,500);
    }
    catch(cv::Exception &e)
    {
        if(QMessageBox::critical(this,"error",e.what()))  return false;
    }
    if(detecteLines.size() == 0)
    {
        QMessageBox::warning(this,"提示","未检测到直线",QMessageBox::Yes);
        return false;
    }
    p->paintHoughResult(this->image_BGR_display,detecteLines);
    ui->displayLabel->setPixmap(QPixmap::fromImage(p->cvMat2QImage(this->image_BGR_display)));
    return true;
}

bool MainWindow::detect_point(cv::Rect rect)
{
    //制作十字模板 模板最小为50 px
    int templeWidth = std::max(std::min(rect.width/4,rect.height/4),50);
    cv::Mat templ = cv::Mat::zeros(templeWidth,templeWidth,CV_8UC1);
    cv::line(templ,cv::Point(templeWidth/2,0),cv::Point(templeWidth/2,templeWidth),cv::Scalar(255),3);
    cv::line(templ,cv::Point(0,templeWidth/2),cv::Point(templeWidth,templeWidth/2),cv::Scalar(255),3);
    Result_Templatematch matchResult;
//    cv::Mat image;
//    cv::cvtColor(ui->displayLabel->QImageToMat(ui->displayLabel->pixmap()->toImage()), image, cv::COLOR_RGBA2GRAY);
//    cv::Mat roi = image(rect);
    cv::Mat roi;
    try
    {
        cv::Mat image_gray = this->image_GRAY.clone();
        roi = image_gray(rect);
        p->gaussianBlur(roi);
        p->enhanceContrast(roi);
        p->binarization(roi,15);
        cv::imshow("image",roi);
        p->canny(roi);
        p->dilateErode(roi);
        matchResult = p->matchtemplate(roi,templ);
    }catch(cv::Exception &e)
    {
        QMessageBox::critical(this,"error",e.what());
        return false;
    }
    if(matchResult.x == -1)
    {
        ui->plainTextEdit->appendPlainText("未匹配到符合要求的区域");
        return false;
    }
    this->originCenterPos = QPoint(rect.x+matchResult.x+matchResult.cols/2,rect.y+matchResult.y+matchResult.rows/2);
    qDebug()<<"[info] matchPoint:"<<originCenterPos.x()<<" "<<originCenterPos.y();
    cv::circle(image_BGR_display,cv::Point(originCenterPos.x(),originCenterPos.y()),20,cv::Scalar(0,0,255),-1);
    p->start_pos = QPoint(rect.x,rect.y);
    p->paintMatchTemplateResult(this->image_BGR_display,matchResult);
    ui->displayLabel->setPixmap(QPixmap::fromImage(p->cvMat2QImage(this->image_BGR_display)));
    return true;
}

void MainWindow::on_lowLeftButton_pressed()
{qDebug()<<"leftpressed";
    unsigned char speed[3] = {0x02,0x01,0x00};
    unsigned char ch[8] = {0xAA,0x55,0x02,0x01,0x00,0x00,0x00,0x00};
    ch[3] = speed[ui->speedComboBox->currentIndex()];
    if(ui->speedComboBox->currentIndex() == 3){
        ch[2] = 0x03;
        ch[3] = 0x02;
        ch[5] = 0x01*this->singleStep;
    }
    ch[7] = ch[0]+ch[1]+ch[2]+ch[3]+ch[4]+ch[5]+ch[6];
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
    qDebug()<<ch[5];
}

void MainWindow::on_lowRightButton_pressed()
{qDebug()<<"rigthpressed";
    unsigned char speed[3] = {0x02,0x01,0x00};
    unsigned char ch[8] = {0xAA,0x55,0x04,0x01,0x00,0x00,0x00,0x00};
    ch[3] = speed[ui->speedComboBox->currentIndex()];
    if(ui->speedComboBox->currentIndex() == 3){
        ch[2] = 0x05;
        ch[3] = 0x02;
        ch[5] = 0x01*this->singleStep;
    }
    ch[7] = ch[0]+ch[1]+ch[2]+ch[3]+ch[4]+ch[5]+ch[6];
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));

}

void MainWindow::on_lowLeftButton_released()
{qDebug()<<"leftreleased";
    if(ui->speedComboBox->currentIndex() == 3){
        //计算离焦距离值
        this->defocusDistance = -this->udpThread->distance + this->defocusDistanceZero;
        ui->ruleDistanceLineEdit->setText(QString::number(this->defocusDistance));
        return ;
    }
    unsigned char speed[3] = {0x02,0x01,0x00};
    unsigned char ch[8] = {0xAA,0x55,0x06,0x01,0x00,0x00,0x00,0x00};
    ch[3] = speed[ui->speedComboBox->currentIndex()];
    ch[7] = ch[0]+ch[1]+ch[2]+ch[3]+ch[4]+ch[5]+ch[6];
    //发三次避免没有响应
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
    Sleep(250);
    while(this->udpThread->isChange){
        this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
        qDebug()<<"send stop message.";
        Sleep(50);
    }
    //计算离焦距离值
    this->defocusDistance = -this->udpThread->distance + this->defocusDistanceZero;
    ui->ruleDistanceLineEdit->setText(QString::number(this->defocusDistance));

}

void MainWindow::on_lowRightButton_released()
{qDebug()<<"rightreleased";
    if(ui->speedComboBox->currentIndex() == 3){
        //计算离焦距离值
        this->defocusDistance = -this->udpThread->distance + this->defocusDistanceZero;
        ui->ruleDistanceLineEdit->setText(QString::number(this->defocusDistance));
        return ;
    }
    unsigned char speed[3] = {0x02,0x01,0x00};
    unsigned char ch[8] = {0xAA,0x55,0x06,0x01,0x00,0x00,0x00,0x00};
    ch[3] = speed[ui->speedComboBox->currentIndex()];
    ch[7] = ch[0]+ch[1]+ch[2]+ch[3]+ch[4]+ch[5]+ch[6];
    //发三次避免没有响应
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
    Sleep(250);
    while(this->udpThread->isChange){
        this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
        qDebug()<<"send stop message.";
        Sleep(50);
    }
    //计算离焦距离值
    this->defocusDistance = -this->udpThread->distance + this->defocusDistanceZero;
    ui->ruleDistanceLineEdit->setText(QString::number(this->defocusDistance));

}

void MainWindow::on_lowLeftButton_2_pressed()
{
    unsigned char speed[3] = {0x02,0x01,0x00};
    unsigned char ch[8] = {0xAA,0x55,0x02,0x01,0x00,0x00,0x00,0x00};
    ch[3] = speed[ui->speedComboBox_2->currentIndex()];
    if(ui->speedComboBox_2->currentIndex() == 3){
        ch[2] = 0x03;
        ch[3] = 0x02;
        ch[5] = 0x01*this->singleStep;
    }
    ch[7] = ch[0]+ch[1]+ch[2]+ch[3]+ch[4]+ch[5]+ch[6];
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
}

void MainWindow::on_lowLeftButton_2_released()
{
    if(ui->speedComboBox_2->currentIndex() == 3){
        return ;
    }
    unsigned char speed[3] = {0x02,0x01,0x00};
    unsigned char ch[8] = {0xAA,0x55,0x06,0x01,0x00,0x00,0x00,0x00};
    ch[3] = speed[ui->speedComboBox_2->currentIndex()];
    ch[7] = ch[0]+ch[1]+ch[2]+ch[3]+ch[4]+ch[5]+ch[6];
    //发三次避免没有响应
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
    Sleep(250);
    while(this->udpThread->isChange){
        this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
        qDebug()<<"send stop message.";
        Sleep(50);
    }
}

void MainWindow::on_lowRightButton_2_pressed()
{
    unsigned char speed[3] = {0x02,0x01,0x00};
    unsigned char ch[8] = {0xAA,0x55,0x04,0x01,0x00,0x00,0x00,0x00};
    ch[3] = speed[ui->speedComboBox_2->currentIndex()];
    if(ui->speedComboBox_2->currentIndex() == 3){
        ch[2] = 0x05;
        ch[3] = 0x02;
        ch[5] = 0x01*this->singleStep;
    }
    ch[7] = ch[0]+ch[1]+ch[2]+ch[3]+ch[4]+ch[5]+ch[6];
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
}

void MainWindow::on_lowRightButton_2_released()
{
    if(ui->speedComboBox_2->currentIndex() == 3){
        return ;
    }
    unsigned char speed[3] = {0x02,0x01,0x00};
    unsigned char ch[8] = {0xAA,0x55,0x06,0x01,0x01,0x00,0x00,0x00};
    ch[3] = speed[ui->speedComboBox_2->currentIndex()];
    ch[7] = ch[0]+ch[1]+ch[2]+ch[3]+ch[4]+ch[5]+ch[6];
    //发三次避免没有响应
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
    Sleep(250);
    while(this->udpThread->isChange){
        this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
        qDebug()<<"send stop message.";
        Sleep(50);
    }
}

void MainWindow::on_resultLineEdit_4_returnPressed()
{
    if(ui->productIDLineEdit1->text().isEmpty())
    {
        QMessageBox::warning(this,"提示","请输入产品编号",QMessageBox::Yes);
        return ;
    }
    ui->tabWidget_2->setCurrentIndex(0);
    QString text = ui->resultLineEdit_4->text();
    double value = text.toDouble();
    int index = ui->detectItemComboBox->currentIndex();
    if(index > 12 && index != 18 && index != 19 && index != 15)
    {
        this->ndetectResult = value;
        this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1);
        nCurrentCount++;
    }
    if(index == 15){
        if(ui->minuteLineEdit->text().isEmpty()){
            QMessageBox::warning(this,"提示","左极限不能为空",QMessageBox::Yes);
            return;
        }
        double valueMinute = ui->minuteLineEdit->text().toDouble();
        if(valueMinute > value){
            QMessageBox::warning(this,"错误","左极限必须不大于右极限",QMessageBox::Yes);
            return;
        }
        this->ndetectResult = valueMinute;
        this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1) ;
        this->ndetectResult_vaxis = value;
        this->aveDetectResult_vaxis = (aveDetectResult_vaxis * nCurrentCount + ndetectResult_vaxis)/(double)(nCurrentCount +1) ;
        nCurrentCount++;
    }
    if(index == 18 || index == 19){
        ndetectResult = value;
        if(index == 18) leftRateArray[nCurrentCount] = ndetectResult;
        if(index == 19) rightRateArray[nCurrentCount] = ndetectResult;
        this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1) ;
        nCurrentCount++;
    }
    printCurrentDetectResult();
    if(nCurrentCount == this->testCount)
    {
        if(index == 13)
        {
            this->leftExitPupil = QString::number(this->aveDetectResult)+"mm";
            ui->leftExitPupilLineEdit->setText(this->leftExitPupil);
        }
        else if(index == 14)
        {
            this->rightExitPupil = QString::number(this->aveDetectResult)+"mm";
            ui->rightExitPupilLineEdit->setText(this->rightExitPupil);
        }
        else if(index == 15)
        {
            this->pupilDistanceRange = "["+QString::number(this->aveDetectResult)+" "+QString::number(this->aveDetectResult_vaxis)+"]"+"mm";
            ui->pupilDistanceRangeLineEdit->setText(this->pupilDistanceRange);
        }
        else if(index == 16)
        {
            this->leftExitPupilDistance = QString::number(this->aveDetectResult)+"mm";
            ui->leftExitPupilDistanceLineEdit->setText(this->leftExitPupilDistance);
        }
        else if(index == 17)
        {
            this->rightExitPupilDistance = QString::number(this->aveDetectResult)+"mm";
            ui->rightExitPupilDistanceLineEdit->setText(this->rightExitPupilDistance);
        }
        else if(index == 18)
        {
            this->leftRate = QString::number(this->aveDetectResult)+"X";
            ui->leftRateLineEdit->setText(this->leftRate);
        }
        else if(index == 19)
        {
            this->rightRate = QString::number(this->aveDetectResult)+"X";
            ui->rightRateLineEdit->setText(this->rightRate);
        }
        else if(index == 23)
        {
            this->leftDistortion = QString::number(this->aveDetectResult)+"%";
            ui->leftDistortionLineEdit->setText(this->leftDistortion);
        }
        else if(index == 24)
        {
            this->rightDistortion = QString::number(this->aveDetectResult)+"%";
            ui->rightDistortionLineEdit->setText(this->rightDistortion);
        }
        //保存图片
        QImage img((uchar*)image_BGR_display.data,image_BGR_display.cols,image_BGR_display.rows,image_BGR_display.cols * 3,QImage::Format_RGB888);
        QDir dir;
        QString dirPath = QString("./assets/%1").arg(this->productID);
        if(dir.mkpath(dirPath))
        {   //图片格式是png会出错，估计是压缩了
            QString save_path = QDir(dirPath).path()+"/"+QString("%2.%3").arg(this->itemID[this->currentItemIndex]).arg(this->saveFormat);
            img.save(save_path,const_cast<const char*>((char*)this->saveFormat.toUtf8().data()),100);
        }
        nCurrentCount = 0;
        if(!this->finishItemFlags[index])
        {
            this->finishItemFlags[index] = true;
            finishItemCount++;
        }
        //tabwidget2 转换到resulttab
        ui->tabWidget_2->setCurrentIndex(1);
    }
    //进度条+1  step=2
    myProgressBarStepUpdate(2);
}

void MainWindow::itemLayout(int index)
{
    //布局
    //产品类型、检测项目
    ui->detectItemComboBox->setCurrentIndex(index);
    ui->productComboBox->setCurrentIndex(this->currentProductType);  //不会激活activate相应函数
    // 分划倾斜 像倾斜 光轴平行度
    if(index == 6 || index == 7 || index == 8 || index == 10)
    {
        //关闭参数设置
        ui->groupBox_3->setVisible(false);
        //打开光栅尺、图像采集、指标检测/图像分析
        ui->groupBox_4->setVisible(true);
        ui->groupBox_11->setVisible(true);
        ui->groupBox_8->setVisible(true);
        ui->getImageButton->setText(tr("获取图片"));
        ui->groupBox_6->setVisible(true);
        ui->imageProcessButton->setVisible(true);
        //关闭检测结果
        ui->groupBox_9->setVisible(false);
        //关闭分辨力
        ui->groupBox_10->setVisible(false);
        ui->groupBox_12->setVisible(false);
        //光轴平行度标志
        this->isMatchRightPoint = false;
    }
    //相对像倾斜、倍率差
    else if(index == 9 || index == 20){
        //关闭参数设置
        ui->groupBox_3->setVisible(false);
        ui->groupBox_4->setVisible(false);
        ui->groupBox_11->setVisible(false);
        ui->groupBox_8->setVisible(false);
        //打开指标计算
        ui->groupBox_6->setVisible(true);
        ui->imageProcessButton->setVisible(false);
        ui->indicatorComputeButton->setEnabled(true);
        //关闭检测结果
        ui->groupBox_9->setVisible(false);
        //关闭分辨力
        ui->groupBox_10->setVisible(false);
        ui->groupBox_12->setVisible(false);
    }
    //视差
    else if(index == 0 || index == 1)
    {
        //参数设置  设置瞳距直径、放大率
        ui->groupBox_3->setVisible(true);
        ui->label_6->setVisible(true);
        ui->PDLineEdit_36->setVisible(true);
        ui->label_11->setVisible(true);
        ui->label_7->setVisible(true);
        ui->rateLineEdit_5->setVisible(true);
        ui->label_13->setVisible(true);
        ui->label_3->setVisible(true);
        ui->focusLineEdit_6->setVisible(true);
        ui->label_14->setVisible(true);
        ui->label_9->setVisible(true);
        ui->P2LineEdit_7->setVisible(true);
        ui->label_15->setVisible(true);
        //打开光栅尺、图像采集、指标检测  关闭图像分析
        ui->groupBox_4->setVisible(true);
        ui->groupBox_11->setVisible(true);
        ui->groupBox_8->setVisible(true);
        ui->getImageButton->setText(tr("停止采集"));
        ui->groupBox_6->setVisible(true);
        ui->imageProcessButton->setVisible(false);
        //关闭检测结果
        ui->groupBox_9->setVisible(false);
        //关闭分辨力
        ui->groupBox_10->setVisible(false);
        ui->groupBox_12->setVisible(false);
    }
    //视度调节范围
    else if(index == 2 ||index == 3)
    {
        //打开参数设置
        ui->groupBox_3->setVisible(true);
        ui->label_6->setVisible(false);
        ui->PDLineEdit_36->setVisible(false);
        ui->label_11->setVisible(false);
        ui->label_7->setVisible(false);
        ui->rateLineEdit_5->setVisible(false);
        ui->label_13->setVisible(false);
        ui->label_3->setVisible(true);
        ui->focusLineEdit_6->setVisible(true);
        ui->label_14->setVisible(true);
        ui->label_9->setVisible(true);
        ui->P2LineEdit_7->setVisible(true);
        ui->label_15->setVisible(true);
        //打开光栅尺、图像采集、指标检测
        ui->groupBox_4->setVisible(true);
        ui->groupBox_11->setVisible(true);
        ui->groupBox_8->setVisible(true);
        ui->getImageButton->setText(tr("停止采集"));
        ui->groupBox_6->setVisible(false);
        ui->groupBox_9->setVisible(false);
        //关闭分辨力
        ui->groupBox_10->setVisible(false);
        ui->groupBox_12->setVisible(true);

    }
    //视度零位
    else if(index == 4 || index == 5)
    {
        //参数设置  设置镜头焦距、P2
        ui->groupBox_3->setVisible(true);
        ui->label_6->setVisible(false);
        ui->PDLineEdit_36->setVisible(false);
        ui->label_11->setVisible(false);
        ui->label_7->setVisible(false);
        ui->rateLineEdit_5->setVisible(false);
        ui->label_13->setVisible(false);
        ui->label_3->setVisible(true);
        ui->focusLineEdit_6->setVisible(true);
        ui->label_14->setVisible(true);
        ui->label_9->setVisible(true);
        ui->P2LineEdit_7->setVisible(true);
        ui->label_15->setVisible(true);
        //打开光栅尺、图像采集、指标检测
        ui->groupBox_4->setVisible(true);
        ui->groupBox_11->setVisible(true);
        ui->groupBox_8->setVisible(true);
        ui->getImageButton->setText(tr("停止采集"));
        ui->groupBox_6->setVisible(true);
        ui->imageProcessButton->setVisible(false);
        //关闭检测结果
        ui->groupBox_9->setVisible(false);
        //关闭分辨力
        ui->groupBox_10->setVisible(false);
        ui->groupBox_12->setVisible(false);
    }
    //分辨力
    else if(index == 11 || index == 12)
    {
        //关闭参数设置
        ui->groupBox_3->setVisible(false);
        //开光栅尺、图像采集
        ui->groupBox_4->setVisible(true);
        ui->groupBox_11->setVisible(true);
        ui->groupBox_8->setVisible(true);
        ui->getImageButton->setText(tr("停止采集"));
        ui->groupBox_6->setVisible(false);
        //关闭检测结果
        ui->groupBox_9->setVisible(false);
        //打开分辨力
        ui->groupBox_10->setVisible(true);
        ui->groupBox_10->setEnabled(false);
        ui->groupBox_12->setVisible(false);
        ui->unitLineEdit_4->clear();
    }
    else
    {
        //只提供一个结束输入框
        ui->groupBox_3->setVisible(false);
        ui->groupBox_4->setVisible(false);
        ui->groupBox_6->setVisible(false);
        ui->groupBox_8->setVisible(false);
        ui->groupBox_11->setVisible(false);
        //打开检测结果
        ui->groupBox_9->setVisible(true);
        ui->resultLineEdit_4->setVisible(true);
        ui->resultLineEdit_4->clear();
        ui->resultUnitLabel->setVisible(true);
        if(index <= 17){
            ui->resultUnitLabel->setText(" mm");
        }
        else if(index == 18 || index == 19){
            ui->resultUnitLabel->setText(" X");
        }
        else if(index == 23 || index == 24){
            ui->resultUnitLabel->setText(" %");
        }
        ui->degreeLabel->setVisible(false);
        ui->degreeLineEdit->setVisible(false);
        ui->minuteLabel->setVisible(false);
        ui->minuteLineEdit->setVisible(false);
        //关闭分辨力
        ui->groupBox_10->setVisible(false);
        ui->groupBox_12->setVisible(false);
    }
    //瞳距适应范围
    if(index == 15){
        ui->minuteLabel->setVisible(true);
        ui->minuteLineEdit->setVisible(true);
        ui->minuteLabel->setText("-");

    }
    //视场输入
    if(index == 21 || index == 22){
        ui->resultLineEdit_4->setVisible(false);
        ui->resultUnitLabel->setVisible(false);
        ui->degreeLabel->setVisible(true);
        ui->degreeLineEdit->setVisible(true);
        ui->minuteLabel->setVisible(true);
        ui->minuteLabel->setText("\u2032");
        ui->minuteLineEdit->setVisible(true);
        ui->groupBox_12->setVisible(false);
    }
    //调整进度条
    myProgressBarInit(index);
    //使能光栅尺
    ui->groupBox_4->setEnabled(false);
}



void MainWindow::on_PDLineEdit_36_returnPressed()
{
    if(ui->PDLineEdit_36->text().isEmpty())
    {
        this->pExitPupilDistance = 0;
    }
    else
    {
        //text不为数字为0
        this->pExitPupilDistance = ui->PDLineEdit_36->text().toDouble();
    }
    ui->PDLineEdit_36->setText(QString::number(this->pExitPupilDistance));
    //进度条+1  step=2
    myProgressBarStepUpdate(2);
}

void MainWindow::on_rateLineEdit_5_returnPressed()
{
    if(ui->rateLineEdit_5->text().isEmpty())
    {
        this->pMagnification = 0;
    }
    else
    {
        //text不为数字为0
        this->pMagnification = ui->rateLineEdit_5->text().toDouble();
    }
    ui->rateLineEdit_5->setText(QString::number(this->pMagnification));
    //进度条+1  step=2
    myProgressBarStepUpdate(2);
}

void MainWindow::on_focusLineEdit_6_returnPressed()
{
    if(ui->focusLineEdit_6->text().isEmpty())
    {
        this->lenFocus = 0;
    }
    else
    {
        //text不为数字为0
        this->lenFocus = ui->focusLineEdit_6->text().toDouble();
    }
    ui->focusLineEdit_6->setText(QString::number(this->lenFocus));
    //进度条+1  step=2
    myProgressBarStepUpdate(2);
}

void MainWindow::on_P2LineEdit_7_returnPressed()
{
    if(ui->P2LineEdit_7->text().isEmpty())
    {
        this->pP2 = 0;
    }
    else
    {
        //text不为数字为0
        this->pP2 = ui->P2LineEdit_7->text().toDouble();
    }
    ui->P2LineEdit_7->setText(QString::number(this->pP2));
    //进度条+1  step=2
    myProgressBarStepUpdate(2);
}

void MainWindow::on_unitLineEdit_4_returnPressed()
{
    if(ui->productIDLineEdit1->text().isEmpty())
    {
        QMessageBox::warning(this,"提示","请输入产品编号",QMessageBox::Yes);
        return ;
    }
    int unitID = ui->unitLineEdit_4->text().toInt();
    if(!unitID || unitID >25)
    {
        QMessageBox::critical(this,"error","输入单元号错误，输入值应在1-25之间");
        return;
    }
    ui->tabWidget_2->setCurrentIndex(0);
    int boardID = ui->resolutionBoardComboBox->currentIndex();
    if(this->currentItemIndex == 11 || this->currentItemIndex == 12)
    {
        QString value = this->resolutionValue[boardID][unitID-1];
        QStringList valueList = value.split("\u2033");
        this->ndetectResult = QString(valueList[0]).toDouble();
        this->ndetectResult_vaxis = QString(valueList[1]).toDouble();
        this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1) ;
        this->aveDetectResult_vaxis = (aveDetectResult_vaxis * nCurrentCount + ndetectResult_vaxis)/(double)(nCurrentCount +1) ;
        nCurrentCount++;
    }
    printCurrentDetectResult();
    if(nCurrentCount == this->testCount)
    {
        if(currentItemIndex == 11)
        {
            this->leftResolution = QString::number(static_cast<int>(aveDetectResult)) + "\u2033" + QString::number(static_cast<int>(aveDetectResult_vaxis)).rightJustified(2,'0');
            ui->leftResolutionLneEdit->setText(leftResolution);
        }
        else if(currentItemIndex ==12)
        {
            this->rightResolution = QString::number(static_cast<int>(aveDetectResult)) + "\u2033" + QString::number(static_cast<int>(aveDetectResult_vaxis)).rightJustified(2,'0');
            ui->rightResolutionLineEdit->setText(rightResolution);
        }
        //保存图片
        QImage img((uchar*)image_BGR_display.data,image_BGR_display.cols,image_BGR_display.rows,image_BGR_display.cols * 3,QImage::Format_RGB888);
        QDir dir;
        QString dirPath = QString("./assets/%1").arg(this->productID);
        if(dir.mkpath(dirPath))
        {   //图片格式是png会出错，估计是压缩了
            QString save_path = QDir(dirPath).path()+"/"+QString("%2.%3").arg(this->itemID[this->currentItemIndex]).arg(this->saveFormat);
            img.save(save_path,const_cast<const char*>((char*)this->saveFormat.toUtf8().data()),100);
        }
        nCurrentCount = 0;
        if(!this->finishItemFlags[currentItemIndex])
        {
            this->finishItemFlags[currentItemIndex] = true;
            finishItemCount++;
        }
        //跳转之resultTab
        ui->tabWidget_2->setCurrentIndex(1);
    }
    //进度条+1  step=3
    myProgressBarStepUpdate(3);
}


void MainWindow::on_minuteLineEdit_returnPressed()
{
    if(ui->productIDLineEdit1->text().isEmpty())
    {
        QMessageBox::warning(this,"提示","请输入产品编号",QMessageBox::Yes);
        return ;
    }
    int index = ui->detectItemComboBox->currentIndex();
    if(index != 21 && index != 22){
        return ;
    }
    if(ui->degreeLineEdit->text().isEmpty() || ui->minuteLineEdit->text().isEmpty()){
        QMessageBox::warning(this,"error","度和分不能为空！");
        return;
    }
    ui->tabWidget_2->setCurrentIndex(0);
    double degree = ui->degreeLineEdit->text().toDouble();
    double minute = ui->minuteLineEdit->text().toDouble();
    if(index == 21 || index == 22)
    {
        this->ndetectResult = degree;
        this->ndetectResult_vaxis = minute;
        this->aveDetectResult = (aveDetectResult * nCurrentCount + ndetectResult)/(double)(nCurrentCount +1);
        this->aveDetectResult_vaxis = (aveDetectResult_vaxis * nCurrentCount + ndetectResult_vaxis)/(double)(nCurrentCount +1);
        nCurrentCount++;
    }
    printCurrentDetectResult();
    if(nCurrentCount == this->testCount)
    {
        QString str = QString::number(this->aveDetectResult) + "\u00B0" +  QString::number(this->aveDetectResult_vaxis) + "\u2032";
        if(index == 21)
        {
            this->leftField = str;
            ui->leftFieldLineEdit->setText(this->leftField);
        }
        else if(index == 22)
        {
            this->rightField = str;
            ui->rightFieldLineEdit->setText(this->rightField);
        }
        //保存图片
        QImage img((uchar*)image_BGR_display.data,image_BGR_display.cols,image_BGR_display.rows,image_BGR_display.cols * 3,QImage::Format_RGB888);
        QDir dir;
        QString dirPath = QString("./assets/%1").arg(this->productID);
        if(dir.mkpath(dirPath))
        {   //图片格式是png会出错，估计是压缩了
            QString save_path = QDir(dirPath).path()+"/"+QString("%2.%3").arg(this->itemID[this->currentItemIndex]).arg(this->saveFormat);
            img.save(save_path,const_cast<const char*>((char*)this->saveFormat.toUtf8().data()),100);
        }
        nCurrentCount = 0;
        if(!this->finishItemFlags[index])
        {
            this->finishItemFlags[index] = true;
            finishItemCount++;
        }
        //跳转之resultTab
        ui->tabWidget_2->setCurrentIndex(1);
    }
    //进度条+1  step=2
    myProgressBarStepUpdate(2);
}

void MainWindow::on_focusConfirmButton_2_clicked()
{
    this->defocusDistanceZero = this->udpThread->distance;
    ui->ruleDistanceLineEdit->setText("0");

}


void MainWindow::on_productComboBox_activated(int index)
{
    this->currentProductType = index;
    if(index == 0){
        this->pExitPupilDistance = 50;
        this->pMagnification = 7;
        this->lenFocus = 50;
        this->pP2 = 32;
        ui->PDLineEdit_36->setText(QString::number(this->pExitPupilDistance));
        ui->rateLineEdit_5->setText(QString::number(this->pMagnification));
        ui->focusLineEdit_6->setText(QString::number(this->lenFocus));
        ui->P2LineEdit_7->setText(QString::number(this->pP2));
    }
    else if(index == 1){
        this->pExitPupilDistance = 40;
        this->pMagnification = 7;
        this->lenFocus = 50;
        this->pP2 = 32;
        ui->PDLineEdit_36->setText(QString::number(this->pExitPupilDistance));
        ui->rateLineEdit_5->setText(QString::number(this->pMagnification));
        ui->focusLineEdit_6->setText(QString::number(this->lenFocus));
        ui->P2LineEdit_7->setText(QString::number(this->pP2));
    }
    else if(index == 2){
        this->pExitPupilDistance = 30;
        this->pMagnification = 6;
        this->lenFocus = 50;
        this->pP2 = 32;
        ui->PDLineEdit_36->setText(QString::number(this->pExitPupilDistance));
        ui->rateLineEdit_5->setText(QString::number(this->pMagnification));
        ui->focusLineEdit_6->setText(QString::number(this->lenFocus));
        ui->P2LineEdit_7->setText(QString::number(this->pP2));
    }
}

void MainWindow::on_BarLeftButton_clicked()
{
    myProgressBarStepUpdate(this->step - 1);
}

void MainWindow::on_BarRightButton_clicked()
{
    myProgressBarStepUpdate(this->step + 1);
}

void MainWindow::on_zeroButton_clicked()
{
    unsigned char speed[3] = {0x02,0x01,0x00};
    unsigned char ch[8] = {0xAA,0x55,0x03,0x02,0x00,0x0A,0x00,0x00};
    ch[3] = speed[ui->speedComboBox->currentIndex()];
    ch[7] = ch[0]+ch[1]+ch[2]+ch[3]+ch[4]+ch[5]+ch[6];
    double motorStepPrecision = 0.02;
    uint16_t motorStep = 1;
    motorStep = std::round(abs(this->defocusDistance)/motorStepPrecision);
    qDebug()<<motorStep<<" "<<this->defocusDistance;
    if(motorStep == 0) return;
    if(this->defocusDistance>0){
        ch[2] = 0x05;
    }
    else{
        ch[2] = 0x03;
    }
    ch[4] = (motorStep>>8) & 0xFF;
    ch[5] = motorStep & 0xFF;
    ch[7] = ch[0]+ch[1]+ch[2]+ch[3]+ch[4]+ch[5]+ch[6];
//    qDebug()<<QString::number(motorStep,16)<<" "<<QString::number(ch[4],16)<<" "<<QString::number(ch[5],16);
    this->udpThread->writeDatagram(QByteArray::fromRawData((char*)ch,8));
    this->defocusDistance = -this->udpThread->distance + this->defocusDistanceZero;
    ui->ruleDistanceLineEdit->setText(QString::number(this->defocusDistance));

}
