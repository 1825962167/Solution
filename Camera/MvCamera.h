#ifndef MVCAMERA_H
#define MVCAMERA_H

#include <QObject> // 使用信号槽机制
#include <QDebug>
#include <string>
#include <vector>
#include <QString>
#include "MvCameraControl.h"
#include "Windows.h" //HWND
#include "opencv2/core.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// 自己定义的错误码 0x0x80009000 - 0x80009999
#define MV_NO_DEVICE_FOUND 0x80009000

// 操作海康相机的类
class MvCamera : public QObject
{
    Q_OBJECT

public:
    MvCamera();
    ~MvCamera();

    // 相机 - 基本操作
    static int EnumDevices(MV_CC_DEVICE_INFO_LIST *m_DeviceList);
    int Open(MV_CC_DEVICE_INFO *pstDeviceInfo); // 打开设备
    int Close();                                // 关闭设备
    int StartGrabbing();                        // 开启抓图
    int StopGrabbing();                         // 停止抓图

    // 相机 - 参数设置
    int GetTriggerMode(unsigned int *returnTriggerMode);  // 获取触发模式
    int SetTriggerMode(unsigned int tobeset_TriggerMode); // 设置触发模式
    int GetExposureTime(float *returnExposureTime);       // 获取曝光时间
    int SetExposureTime(float tobeset_ExposureTime);      // 设置曝光时间
    int SetExposureTimeAuto();                            //设置自动曝光时间
    int CloseExposureTimeAuto();                          //关闭自动曝光时间
    int GetGain(float *returnGain);                       // 获取增益
    int SetGain(float tobeset_Gain);                      // 设置增益
    int SetGainAuto();                                    // 设置自动增益
    int CloseGainAuto();                                  // 关闭自动增益
    int GetResultingFrameRate(float *returnFrameRate);    // 获取实际帧率
    int SetResultingFrameRate(float tobeset_frameRate);   // 设置实际帧率
    int GetAcquisitionFrameRate(float *returnFrameRate);  //获取理论帧率
    int SetAcquisitionFrameRate(float tobeset_frameRate); //设置理论帧率
    // 相机 - 获取帧/释放帧/显示帧
    int GetImageBuffer(MV_FRAME_OUT *pFrame, int nMsec); // 主动获取一帧图像数据
    int FreeImageBuffer(MV_FRAME_OUT *pFrame);           // 释放图像缓存
    int GetImageFOrBGR(unsigned char *pData,unsigned int nDataSize,MV_FRAME_OUT_INFO_EX *pstFrameInfo,int nMsec);//查询式获取一帧数据
    int DisplayOneFrame(MV_DISPLAY_FRAME_INFO *pDisplayInfo);

    //转换像素格式 输出:Mat  调用MV_CC_ConvertPixelType接口 也可以直接cvtcolor转换
    int ConvertPixelTypeToGray(MV_FRAME_OUT *pFrame,cv::Mat& image);
    int ConvertPixelTypeToRGB(MV_FRAME_OUT *pFrame,cv::Mat& image);


signals:
    void s_StartGrabing(); // 传递给不断抓取图像帧的线程，期望其开启线程
    void s_StopGrabing();  // 传递给不断抓取图像帧的线程，期望其关闭线程

public: // private member function
    // 获取Int型参数
    int GetIntValue(IN const char *strKey, OUT unsigned int *pnValue);
    int SetIntValue(IN const char *strKey, IN unsigned int nValue);
    // 获取Float型参数
    int GetFloatValue(IN const char *strKey, OUT float *pfValue);
    int SetFloatValue(IN const char *strKey, IN float fValue);
    // 获取Enum型参数
    int GetEnumValue(IN const char *strKey, OUT unsigned int *pnValue);
    int SetEnumValue(IN const char *strKey, IN unsigned int nValue);
    // 获取Bool型参数
    int GetBoolValue(IN const char *strKey, OUT bool *pbValue);
    int SetBoolValue(IN const char *strKey, IN bool bValue);
    // 获取String型参数
    int GetStringValue(IN const char *strKey, IN OUT char *strValue, IN unsigned int nSize);
    int SetStringValue(IN const char *strKey, IN const char *strValue);


private: // private member
    void *m_DeviceHandle;


    // 相机参数记录
    bool m_bOpenDevice;    // 是否打开设备
    bool m_bStartGrabbing; // 是否开始抓图

    bool m_bSoftWareTriggerCheck;
    unsigned int m_nTriggerMode;   // 触发模式
    unsigned int m_nTriggerSource; // 触发源

    float m_dExposureEdit;  // 曝光值
    float m_dGainEdit;      // 增益值
    float m_dFrameRateEdit; // 实际帧率值
    float m_lFrameRateEdit; //理论帧率值
};

#endif // MVCAMERA_H
