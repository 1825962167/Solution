#ifndef IMAGEPRO_H
#define IMAGEPRO_H

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <vector>
#include "lsdlines.h"
#include <QPoint>
#include <QImage>
#include <cmath>


/*
 to do :
    1、模板匹配添加尺度
    2、image_preprocess参数调试
       将高斯滤波改成双边高斯滤波
    3、模板检测结果显示
 */

//模板匹配输出结果包括匹配区域左上点在原图上的坐标x,y,旋转角度，置信度,模板匹配原图的宽、高
class Result_Templatematch
{
public:
    Result_Templatematch(){}
    Result_Templatematch(int a,int b ,double c,double d,int e ,int f ):x(a),y(b),angle(c),score(d),cols(e),rows(f)
    {

    }
    int x ,y;
    double angle;
    double score;
    int cols;
    int rows;
};

class ImagePro
{
public:
    ImagePro();
    ~ImagePro();

    /**
     * @brief cvMat2QImage Mat(三通道或者一通道)转QImage
     * @param cvImg 输入Mat
     * @return QImage
     */
    QImage cvMat2QImage(const cv::Mat& cvImg);

    void set(cv::Mat &imagel,cv::Mat &imager);
    void set_left(cv::Mat &imagel);
    void set_right(cv::Mat &imager);
    void set_model(cv::Mat &image);

    //image preprocess 
    cv::Mat image_preprocess(cv::Mat image);

    /**
     * @brief gaussianBlur 高斯模糊滤波
     * @param IN&OUT image_gray 灰度图像  kenel_size 卷积核大小
     */
    void gaussianBlur(cv::Mat &image_gray,int kenel_size = 5);

    /**
     * @brief enhanceContrast 提高图像的对比度(归一化、均衡化)
     * @param IN&OUT image_gray 灰度图像
     */
    void enhanceContrast(cv::Mat &image_gray);

    /**
     * @brief binarization
     * @param IN&OUT image_gray 灰度图像 cv::threshold 低阈值 low_thresh 高阈值 high_thresh
     */
    void binarization(cv::Mat &image_gray,int low_thresh = 160,int high_thresh = 255);

    /**
     * @brief canny 边缘检测
     * @param IN&OUT binary_image 二值图像 cv::Canny 低阈值 low_thresh 高阈值 high_thresh
     */
    void canny(cv::Mat &binary_image,int low_thresh = 50,int high_thresh = 150);

    /**
     * @brief  dilateErode 膨胀腐蚀
     * @param  IN&OUT binary_image 二值图像 dilateKenelSize  膨胀卷积核 erodeKernelSize 腐蚀卷积核
     */
    void dilateErode(cv::Mat &binary_image,int dilateKenelSize = 7,int erodeKernelSize = 3);

    /**
     * @brief  imageSharpness 计算采集图像清晰度
     * @param  image_gary 灰度图像
     * @return float 清晰度
     */
    float imageSharpness(const cv::Mat &image_gray,cv::Rect mask = cv::Rect());

    //模板匹配
    //模板匹配参数设置
    void set_thershold_rotation(int thershold_rotation);
    void set_thershold_scale(int thershold_scale);
    void set_matchmode(int mode);
    void set_angle_precision(double precision);
    void set_thershold_score_matchtemplate(double score);
    void set_downsamplelevels(int level=1);
    /**
     * @brief  downsample 对灰度图像和灰度模板降采样
     * @param  image 灰度图像 model 灰度模板  levels 降采样层数
     */
    void downsample(cv::Mat& image ,cv::Mat& model,int levels = 3);

    /**
     * @brief  _rotate_image 图片旋转(旧版) 图片尺寸不变化，模板不完整
     * @param  image 模板边缘图像 OUT newImg 旋转后新模板 angle 旋转角度
     */
    void _rotate_image(cv::Mat& image,cv::Mat& newImg,double angle);

    /**
     * @brief  rotate_image 图片旋转 图片尺寸变化，保持模板完整
     * @param  image 模板边缘图像 OUT newImg 旋转后新模板 angle 旋转角度 OUT mask 保证模板完整且不多余的mask
     */
    void rotate_image(cv::Mat& image,cv::Mat& newImg,double angle,cv::Mat& mask);

    /**
     * @brief  matchtemplate 模板匹配
     * @param  IN&OUT image 二值图像 model 灰度模板
     * @return image上绘制匹配区域 Result_Templatematch 输出结果包括匹配区域左上点在原图上的坐标x,y、旋转角度、置信度、模板匹配区域在原图上的宽,高
     */
    Result_Templatematch matchtemplate(cv::Mat &image ,cv::Mat &model);
//    void plot_matchresult(Result_Templatematch result,cv::Mat& image,cv::Mat& dst);

    /**
     * @brief  hough houghP直线检测
     * @param  binary_image 二值图像 thershold 置信度阈值 minLineLength 最小线宽 maxLineGap 最大线间隔
     * @return std::vector<cv::Vec4i> 直线坐标
     */
    std::vector<cv::Vec4i> hough(cv::Mat &binary_image,int thershold,double minLineLength = 350,double maxLineGap = 20);

    //在彩色图上绘制结果
    /**
     * @brief  paintHoughResult 绘制hough结果 red
     * @param  image_bgr bgr彩色图像   lines hough检测结果 colorvalue 直线颜色 linewidth 线宽
     */
    void paintHoughResult(cv::Mat &image_bgr,std::vector<cv::Vec4i> lines,int colorvalue = 255,int linewidth = 1);

    /**
     * @brief  paintHoughResult 绘制hough结果 blue
     * @param  image_bgr bgr彩色图像   result 模板匹配结果 colorvalue 绘制颜色  linewidth 线宽
     */
    void paintMatchTemplateResult(cv::Mat &image_bgr,Result_Templatematch result,int colorvalue = 255,int linewidth = 3);

    /**
     * @brief  paintElectronicCrossLine 绘制电子十字分划 black
     * @param  IN&OUT image_bgr bgr彩色图像  colorvalue 直线颜色 linewidth  线宽
     */
    void paintElectronicCrossLine(cv::Mat &image_bgr,int colorvalue = 255,int linewidth = 2);

private:
    cv::Mat src_left_,src_right_;        //左右目采样图
    cv::Mat model_;                      //模板匹配模板
    cv::Mat image_GRAY;                  //采集的灰度图像
    cv::Mat image_BGR;                   //采集的bgr图像
    cv::TemplateMatchModes match_mode_ =  cv::TM_CCOEFF_NORMED;   //建议选用相关性的算法 match_mode>1
    int thershold_rotation_=90,thershold_scale_=1;                //默认顺时针和逆时针的[0,180]区间
    double angle_precision_ = 0.01;
    double thershold_score_matchtemplate_ = 0.2;
    int downsample_levels_ = 3;

public:
    //模板匹配结果显示鼠标点击起始位置(displayLabel)
    QPoint start_pos;


};

#endif // IMAGEPRO_H
