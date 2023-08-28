#include "imagepro.h"
#include<iostream>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/core.hpp>
#include<QString>
#include<QDebug>

ImagePro::ImagePro()
{

}

ImagePro::~ImagePro()
{

}

//Mat2QImage   浅拷贝
QImage ImagePro::cvMat2QImage(const cv::Mat& cvImg)
{
    QImage qImg;
    if (cvImg.channels() == 3)                             //三通道彩色图像
    {
        //CV_BGR2RGB
//        cv::cvtColor(cvImg, cvImg, cv::COLOR_RGB2BGR);
        qImg = QImage((const unsigned char*)(cvImg.data), cvImg.cols, cvImg.rows, cvImg.cols*cvImg.channels(), QImage::Format_RGB888);
    }
    else if (cvImg.channels() == 1)                    //单通道（灰度图）
    {
        qImg = QImage((const unsigned char*)(cvImg.data), cvImg.cols, cvImg.rows, cvImg.cols*cvImg.channels(), QImage::Format_Indexed8);

        QVector<QRgb> colorTable;
        for (int k = 0; k<256; ++k)
        {
            colorTable.push_back(qRgb(k, k, k));
        }
        qImg.setColorTable(colorTable);//把qImg的颜色按像素点的颜色给设置
    }
    else
    {
        qImg = QImage((const unsigned char*)(cvImg.data), cvImg.cols, cvImg.rows, cvImg.cols*cvImg.channels(), QImage::Format_RGB888);
    }
//    cvImg;
    return qImg;
}

//输入:灰度图  输出:边缘图
cv::Mat ImagePro::image_preprocess(cv::Mat image)
{
    cv::Mat dst;
    cv::cvtColor(image,dst,cv::COLOR_BGR2RGB);
    cv::cvtColor(dst,dst,cv::COLOR_RGB2GRAY);//将图像转为灰度
    cv::GaussianBlur(dst,dst,cv::Size(5,5),0, 0, cv::BORDER_DEFAULT);//高斯滤波
    cv::Canny(dst,dst,15, 100, 3,true);//参数有待确定？？？
    return dst;
}

void ImagePro::gaussianBlur(cv::Mat &image_gray,int kenel_size)
{
    cv::GaussianBlur(image_gray,image_gray,cv::Size(kenel_size,kenel_size),0,0,cv::BORDER_DEFAULT);
}

void ImagePro::enhanceContrast(cv::Mat &image_gray)
{
   cv::normalize(image_gray, image_gray, 0, 255, cv::NORM_MINMAX);
   cv::equalizeHist(image_gray, image_gray);
}

void ImagePro::binarization(cv::Mat &image_gray,int low_thresh,int high_thresh)
{
    cv::threshold(image_gray,image_gray,low_thresh,high_thresh,cv::THRESH_BINARY);
    image_gray = cv::Scalar(255) - image_gray;
}

void ImagePro::canny(cv::Mat &binary_image,int low_thresh,int high_thresh)
{
    Canny(binary_image,binary_image,low_thresh, high_thresh, 3,true);
}

void ImagePro::dilateErode(cv::Mat &binary_image,int dilateKenelSize,int erodeKernelSize)
{
    cv::Mat kenelDilate = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(dilateKenelSize, dilateKenelSize));
    cv::dilate(binary_image, binary_image, kenelDilate);
    cv::Mat kenelErode = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(erodeKernelSize, erodeKernelSize));
    cv::erode(binary_image, binary_image, kenelErode);
}

float ImagePro::imageSharpness(const cv::Mat &image_gray,cv::Rect mask)
{
    cv::Mat laplacianResult;
    cv::Mat roi;
    if(mask.empty())
    {
        roi = image_gray.clone();
    }
    else
    {
        roi = image_gray(mask);
    }
    cv::Laplacian(roi,laplacianResult,CV_32F);   //对灰度图进行Laplacian变换
    cv::Scalar mean_val, stddev_val;
    cv::meanStdDev(laplacianResult, mean_val, stddev_val); // 计算标准差
    return stddev_val[0];
}

//设置左右目采样图
void ImagePro::set(cv::Mat &imagel,cv::Mat &imager)
{
    if(imagel.empty()||imager.empty())return;
    this->src_left_ = imagel;
    this->src_right_ = imager;
}

//设置左目采样图
void ImagePro::set_left(cv::Mat &imagel)
{
    if(imagel.empty())return;
    this->src_left_ = imagel;
}

//设置右目采样图
void ImagePro::set_right(cv::Mat &imager)
{
    if(imager.empty())return;
    this->src_right_ = imager;
}

//设置模板
void ImagePro::set_model(cv::Mat &image)
{
    if(image.empty())return ;
    this->model_ = image;
}

//设置匹配角度范围
void ImagePro::set_thershold_rotation(int thershold_rotation)
{
    if(thershold_rotation<0 || thershold_rotation>180)
        qDebug()<<QString("角度区间为[0,180]");
    else
        this->thershold_rotation_ = thershold_rotation;
}

//设置尺度阈值
void ImagePro::set_thershold_scale(int thershold_scale)
{
    if(thershold_scale>5)
        qDebug()<<QString("尺寸不宜过大");
    else
        this->thershold_scale_ = thershold_scale;
}

//设置匹配方法
void ImagePro::set_matchmode(int mode)
{
    if(mode>5)
       qDebug()<<QString("error ：超出限制");
    else
        switch (mode)
          {
              case 0:
                this->match_mode_ = cv::TM_SQDIFF;
                break;
              case 1:
                this->match_mode_ = cv::TM_SQDIFF_NORMED;
                break;
              case 2:
                this->match_mode_ = cv::TM_CCORR;
                break;
              case 3:
                this->match_mode_ = cv::TM_CCORR_NORMED;
                break;
              case 4:
                this->match_mode_ = cv::TM_CCOEFF;
                break;
              case 5:
                this->match_mode_ = cv::TM_CCOEFF_NORMED;
                break;
          }
}

//设置降采样的层数
void ImagePro::set_downsamplelevels(int level)
{
    if(level<0)
    {
        qDebug()<<QString("层数不能小于0");
    }
    this->downsample_levels_ = level;
}

//设置角度精度
void ImagePro::set_angle_precision(double precision)
{
    this->angle_precision_ = precision;
}

void ImagePro::set_thershold_score_matchtemplate(double score)
{
    if(score > 1 || score < 0)
        qDebug()<<QString("error: 超出界限");
    else
        this->thershold_score_matchtemplate_ = score;
}

//旋转图片 输入：旋转图片 目标图片 旋转角度 逆时针为正
void ImagePro::_rotate_image(cv::Mat& image,cv::Mat& newImg,double angle)
{
    if(image.empty())
    {
        qDebug()<<QString("输入不能为空");
    }
    cv::Point2f pt = cv::Point2f((float)image.cols / 2, (float)image.rows / 2);
    cv::Mat M = cv::getRotationMatrix2D(pt, angle, 1.0);
    cv::warpAffine(image, newImg, M, image.size(),cv::INTER_CUBIC);
}

//旋转图片 输入：旋转图片 目标图片 旋转角度 mask 逆时针为正
void ImagePro::rotate_image(cv::Mat& image,cv::Mat& newImg,double angle,cv::Mat& mask)
{
    if(image.empty())
    {
        qDebug()<<QString("输入不能为空");
    }
    int w = image.cols;
    int h = image.rows;
    cv::Mat M = cv::getRotationMatrix2D(cv::Point2f((float)w / 2, (float)h / 2), angle, 1.0);//中心点 旋转角度 放缩倍数
    double cos = abs(M.at<double>(0, 0));
    double sin = abs(M.at<double>(0, 1));
    int nw = cos * w + sin * h;
    int nh = sin * w + cos * h;
    M.at<double>(0, 2) += (nw / 2 - w / 2);
    M.at<double>(1, 2) += (nh / 2 - h / 2);
//    qDebug()<<"model size :"<<nw<<" "<<nh;
//    warpAffine(image, newImg, M, Size(nw,nh), 1, 0, Scalar(0, 255, 0));
    cv::warpAffine(image, newImg,M, cv::Size(nw,nh),cv::INTER_LINEAR,cv::BORDER_CONSTANT);
    cv::Mat temp = cv::Mat::zeros(cv::Size(w,h),CV_8UC1);
    temp += 255;//生成全255的Mat
    cv::warpAffine(temp,mask,M,cv::Size(nw,nh),cv::INTER_LINEAR,cv::BORDER_CONSTANT); //生成mask
}

//降采样 输入：匹配图像、模板、金字塔层数(3)
void ImagePro::downsample(cv::Mat& image ,cv::Mat& model,int levels)
{
    if(image.empty())
    {
        qDebug()<<QString("输入不能为空");
    }
    for (int i = 0; i < levels; i++)
    {
      cv::pyrDown(image, image, cv::Size(image.cols / 2, image.rows / 2));
      cv::pyrDown(model, model, cv::Size(model.cols / 2, model.rows / 2));
    }
}

//模板匹配 输入：匹配图像、模板 输出：匹配区域左上点的坐标x,y,旋转角度，置信度
Result_Templatematch ImagePro::matchtemplate(cv::Mat &src ,cv::Mat &src_model)
{
    if(src.empty()||src_model.empty())
    {
        qDebug()<<QString("输入不能为空");
    }
    //init 设置匹配参数
//    set_thershold_rotation(90);
//    set_matchmode(5);
//    set_angle_precision(0.1);
//    set_thershold_score_matchtemplate(0.8);
//    set_downsamplelevels(3);

    cv::Mat image = src.clone();
    cv::Mat model = src_model.clone();
    //1.降采样  默认层数是3
    downsample(image,model,this->downsample_levels_);

    //2.在没有旋转的情况下进行第一次匹配
    cv::Mat newImg;//旋转后模板
    //旋转后模板的类型设置为float，确保精度
    newImg.convertTo(newImg,CV_32F);
    cv::Mat mask;
    double minval, maxval;
    cv::Point minloc, maxloc;
    int result_cols,result_rows;
    cv::Mat result;
    cv::Point location;
    double temp=0,angle=0;
    int return_cols,return_rows;
    double step = 5;//初始默认角步长为5
    int start = 0;
    int range = this->thershold_rotation_;

    //3.以最佳匹配点左右十倍角度步长进行。循环匹配，直到角度步长小于参数角度步长
    //如果匹配方法是TM_SQDIFF TM_SQDIFF_NORMED 就直接返回结果
    if(this->match_mode_ == cv::TM_SQDIFF  || this->match_mode_ == cv::TM_SQDIFF_NORMED)
    {
        temp = 1;
        do
        {
            int num_step = range / step;
            for (int i = -num_step; i <= num_step; i++)
            {
//                rotate_image(model,newImg, start + step * i,mask);
                _rotate_image(model,newImg, start + step * i);
                result_cols = image.cols - newImg.cols +1;
                result_rows = image.rows -newImg.rows + 1;
                result = cv::Mat::zeros(cv::Size(result_cols,result_rows),CV_32FC1);
//                cv::matchTemplate(image, newImg, result, this->match_mode_,mask);
                cv::matchTemplate(image, newImg, result, this->match_mode_);
                cv::minMaxLoc(result, &minval, &maxval, &minloc, &maxloc);
                if (minval < temp)
                {
                    location = minloc;
                    temp = minval;
                    angle = start + step * i;
                    return_cols = newImg.cols;
                    return_rows = newImg.rows;
                }
                qDebug()<<"[rough]angle: "<<angle<<" socre: "<<temp<<" currentsocre: "<<maxval<<" currentangle: "<<start + step * i;
            }
            range = step * 2;
            start = angle;
            step = step / 10;
        } while (step > this->angle_precision_);
        return Result_Templatematch(location.x*pow(2,this->downsample_levels_),location.y*pow(2,this->downsample_levels_),angle,temp,return_cols,return_rows);
//        return ResultPoint(location.x * pow(2, numLevels) + modelImage.cols / 2, location.y * pow(2, numLevels) + modelImage.rows / 2, -angle, temp);
    }
    else
    {
        temp = 0;
        do
        {
            int num_step = range / step;
            for (int i = -num_step; i <= num_step; i++)
            {
//                rotate_image(model,newImg, start + step * i,mask);
                _rotate_image(model,newImg, start + step * i);
                result_cols = image.cols - newImg.cols +1;
                result_rows = image.rows -newImg.rows + 1;
                result = cv::Mat::zeros(cv::Size(result_cols,result_rows),CV_32FC1);
//               cv::matchTemplate(image, newImg, result, this->match_mode_,mask);
                cv::matchTemplate(image, newImg, result, this->match_mode_);
                double minval, maxval;
                cv::Point minloc, maxloc;
                cv::minMaxLoc(result, &minval, &maxval, &minloc, &maxloc);
                if (maxval > temp)
                {
                    location = maxloc;
                    temp = maxval;
                    angle = start + step * i;
                    return_cols = newImg.cols;
                    return_rows = newImg.rows;
                    qDebug()<<QString("(%1,%2)").arg(location.x).arg(location.y);
                }
                qDebug()<<"[rough]angle: "<<angle<<" socre: "<<temp<<" currentsocre: "<<maxval<<" currentangle: "<<start + step * i;
            }
            range = step * 2;
            start = angle;
            step = step / 10.0;
            qDebug()<<"step: "<<step;
        } while (step > this->angle_precision_);
    }

    //4.匹配方法是后四种的话，以原始图重新模板匹配，精确定位
    double k_angle = angle - 10*step;
    cv::Mat model1 = src_model.clone();
    cv::Mat src1 = src.clone();
    //降采样一次
//    cv::pyrDown(src1, src1, cv::Size(src1.cols / 2, src1.rows / 2));
//    rotate_image(model1,newImg,k_angle,mask);
    _rotate_image(model1,newImg,k_angle);
    result_cols = src1.cols - newImg.cols +1;
    result_rows = src1.rows -newImg.rows + 1;
    result = cv::Mat::zeros(cv::Size(result_cols,result_rows),CV_32FC1);
//    matchTemplate(src, newImg, result, this->match_mode_,mask);
    cv::matchTemplate(src1, newImg, result, this->match_mode_);
    cv::minMaxLoc(result,  &minval, &maxval, &minloc, &maxloc);
    location = maxloc;
    temp = maxval;
    angle = k_angle;
    return_cols = newImg.cols;
    return_rows = newImg.rows;
    qDebug()<<"[precise0]angle: "<<angle<<" socre: "<<temp<<" match maxval: "<<maxval<<" currentangle: "<<k_angle;
    int num_step = 2*10*step/this->angle_precision_;
    for(int i = 1;i<num_step;i++)
    {
        k_angle += this->angle_precision_;
//        rotate_image(model1,newImg,k_angle,mask);
        _rotate_image(model1,newImg,k_angle);
        result_cols = src1.cols - newImg.cols +1;
        result_rows = src1.rows -newImg.rows + 1;
        result = cv::Mat::zeros(cv::Size(result_cols,result_rows),CV_32FC1);
//        cv::matchTemplate(src1,newImg,result,this->match_mode_,mask);
        cv::matchTemplate(src1,newImg,result,this->match_mode_);
        cv::minMaxLoc(result,  &minval, &maxval, &minloc, &maxloc);
        if(this->match_mode_!=cv::TM_SQDIFF && this->match_mode_!=cv::TM_SQDIFF_NORMED && maxval>temp)
        {
            location = maxloc;
            temp = maxval;
            angle = k_angle;
            return_cols = newImg.cols;
            return_rows = newImg.rows;
        }
        qDebug()<<"[precise]angle: "<<angle<<" socre: "<<temp<<" currentsocre: "<<maxval<<" angle : "<<k_angle;
    }

    //内存释放
    image.release();
    model.release();
    src1.release();
    model1.release();

    //5.匹配置信度只有大于阈值(0.8)才有效
    //输出结果包括匹配区域左上点的坐标x,y,旋转角度，置信度
    if(temp < this->thershold_score_matchtemplate_)
    {
       qDebug() << QString("未匹配到符合要求的区域");
       return Result_Templatematch(-1, -1, 0, 0, 0, 0);
    }
    else
    {
        Result_Templatematch output(location.x,location.y,angle,temp,return_cols,return_rows);
//        Result_Templatematch output(location.x*pow(2,1),location.y*pow(2,1),angle,temp,return_cols*pow(2,1),return_rows*pow(2,1));
        return output;
    }
}

std::vector<cv::Vec4i> ImagePro::hough(cv::Mat &binary_image,int thershold,double minLineLength,double maxLineGap)
{
   //选择hough变换 HoughLines  HoughLinesP（默认）
    #if 0
    std::vector<cv::Vec2f> lines;
    cv::HoughLines(binary_image, lines, 1, CV_PI/180, 100, 0, 0 );
    //display result
    for( size_t i = 0; i HoughLinesP< lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        cv::Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cv::cvRound(x0 + 1000*(-b));
        pt1.y = cv::cvRound(y0 + 1000*(a));
        pt2.x = cv::cvRound(x0 - 1000*(-b));
        pt2.y = cv::cvRound(y0 - 1000*(a));
        cv::line( dst, pt1, pt2, cv::Scalar(0,0,255), 3, cv::LINE_AA);
    }
    #else
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(binary_image, lines, 1, CV_PI/180, thershold, minLineLength, maxLineGap );
    #endif
   qDebug()<<"[hough] lines number:"<<lines.size();
    return lines;

}

void ImagePro::paintElectronicCrossLine(cv::Mat &image_bgr,int colorvalue,int linewidth)
{
    cv::Point pt1(image_bgr.cols/2,0);
    cv::Point pt2(image_bgr.cols/2,image_bgr.rows);
    cv::Point pt3(0,image_bgr.rows/2);
    cv::Point pt4(image_bgr.cols,image_bgr.rows/2);
    cv::line(image_bgr,pt1,pt2,cv::Scalar(colorvalue,colorvalue,colorvalue),linewidth);
    cv::line(image_bgr,pt3,pt4,cv::Scalar(colorvalue,colorvalue,colorvalue),linewidth);
}

void ImagePro::paintHoughResult(cv::Mat &image_bgr,std::vector<cv::Vec4i> lines,int colorvalue,int linewidth)
{
    double angle = 0;
    double length  = 0;
    for( size_t i = 0; i < lines.size(); i++ )
    {
        cv::Vec4i l = lines[i];
        cv::line(image_bgr, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(colorvalue,0,0), linewidth, cv::LINE_AA);
        angle = atan((double)(l[3]-l[1])/(double)(l[0]-l[2])) * 180 / CV_PI;
        length = cv::norm(cv::Point(l[0], l[1]) - cv::Point(l[2], l[3]));
        qDebug()<<"[hough] line"<<QString("%1").arg(i,3,10,QChar('0'))<<" angle:"<<QString("%1").arg(angle, 0, 'f', 1, '0')<<" length:"<<length;
    }
}

void ImagePro::paintMatchTemplateResult(cv::Mat &image_bgr,Result_Templatematch result,int colorvalue,int linewidth)
{
    cv::rectangle(image_bgr,cv::Rect(cv::Point(this->start_pos.x()+result.x,this->start_pos.y()+result.y),cv::Size(result.cols,result.rows)),cv::Scalar(0,0,colorvalue),linewidth);
}
