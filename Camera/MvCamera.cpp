#include "MvCamera.h"

//using namespace cv;

// 局部函数声明
void PrintDeviceInfo(MV_CC_DEVICE_INFO *pstMVDevInfo)
{
    if (NULL == pstMVDevInfo)
    {
        qDebug("The Pointer of pDeviceInfo is NULL!\n");
    }
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

        // print current ip and user defined name
        qDebug("CurrentIp: %d.%d.%d.%d\n", nIp1, nIp2, nIp3, nIp4);
        qDebug("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        qDebug("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
        qDebug("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
        qDebug("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
    }
    else
    {
        qDebug("Not support.\n");
    }
}


MvCamera::MvCamera()
{
    m_DeviceHandle = nullptr;
}

MvCamera::~MvCamera()
{
    if (m_DeviceHandle)
    {
        MV_CC_DestroyHandle(m_DeviceHandle);
        m_DeviceHandle = nullptr;
    }
}

// 相机 - 基本操作
int MvCamera::EnumDevices(MV_CC_DEVICE_INFO_LIST *m_DeviceList)
{
    // Enum device
    int nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, m_DeviceList);

    if (MV_OK != nRet)
    {
        qDebug("Enum Devices fail! nRet [0x%x]\n", nRet);
    }

    if (m_DeviceList->nDeviceNum > 0)
    {
        // 打印扫描到的设备信息
        for (unsigned int i = 0; i < m_DeviceList->nDeviceNum; i++)
        {
            qDebug("[device %d]:\n", i);
            MV_CC_DEVICE_INFO *pDeviceInfo = m_DeviceList->pDeviceInfo[i];
            if (nullptr == pDeviceInfo)
            {
                break;//这一步有点看不明白
            }
            PrintDeviceInfo(pDeviceInfo); // 局部函数：打印设备信息
        }
        return MV_OK;
    }
    else
    {
        qDebug("No Device Found!");
        return MV_NO_DEVICE_FOUND; // 没有发现设备
    }

}

int MvCamera::Open(MV_CC_DEVICE_INFO *pstDeviceInfo) // 打开设备
{
    if (nullptr == pstDeviceInfo)
    {
        return MV_E_PARAMETER;
    }

    int nRet = MV_OK;
    if (m_DeviceHandle == nullptr)
    {
        nRet = MV_CC_CreateHandle(&m_DeviceHandle, pstDeviceInfo); // 这里需要注意下 handle
        if (MV_OK != nRet)
        {
            return nRet;
        }
    }

    nRet = MV_CC_OpenDevice(m_DeviceHandle);
    if (MV_OK != nRet)
    {
        MV_CC_DestroyHandle(m_DeviceHandle);
        m_DeviceHandle = nullptr;

        return nRet;
    }

    return MV_OK;
}

int MvCamera::Close() // 关闭设备
{
    int nRet = MV_OK;

    if (NULL == m_DeviceHandle)
    {
        return MV_E_PARAMETER; // 返回错误的参数
    }

    MV_CC_CloseDevice(m_DeviceHandle);
    nRet = MV_CC_DestroyHandle(m_DeviceHandle);
    m_DeviceHandle = nullptr;

    return nRet;
}

int MvCamera::StartGrabbing() // 开启抓图
{
    int nRet = MV_CC_StartGrabbing(m_DeviceHandle);

    if (nRet == MV_OK)
    {
        emit s_StartGrabing();
        return MV_OK;
    }

    return nRet;
}

int MvCamera::StopGrabbing() // 停止抓图
{
    int nRet = MV_CC_StopGrabbing(m_DeviceHandle);

    if (nRet == MV_OK)
    {
        emit s_StopGrabing();
        return MV_OK;
    }

    return nRet;
}

// 相机 - 参数获取与设置
int MvCamera::GetTriggerMode(unsigned int *returnTriggerMode) // 获取触发模式
{
    int nRet = GetEnumValue("TriggerMode", &m_nTriggerMode);

    *returnTriggerMode = m_nTriggerMode;
    return nRet;
}

int MvCamera::SetTriggerMode(unsigned int tobeset_TriggerMode) // 设置触发模式
{
    if ((tobeset_TriggerMode != MV_TRIGGER_MODE_ON) | (tobeset_TriggerMode != MV_TRIGGER_MODE_OFF))
        return MV_E_PARAMETER;

    m_nTriggerMode = tobeset_TriggerMode;

    return SetEnumValue("TriggerMode", m_nTriggerMode);
}

int MvCamera::GetExposureTime(float *returnExposureTime) // 获取曝光时间
{
    int nRet = GetFloatValue("ExposureTime", &m_dExposureEdit);

    *returnExposureTime = m_dExposureEdit;
    return nRet;
}

int MvCamera::SetExposureTime(float tobeset_ExposureTime) // 设置曝光时间
{
    if (tobeset_ExposureTime < 0)
    {
        return MV_E_PARAMETER;
    }

    m_dExposureEdit = tobeset_ExposureTime;

    return SetFloatValue("ExposureTime", (float)m_dExposureEdit);
}

int MvCamera::SetExposureTimeAuto()
{
    int nRet = SetEnumValue("ExposureAuto", 2);

    return nRet;
}

int MvCamera::CloseExposureTimeAuto()
{
    int nRet = SetEnumValue("ExposureAuto", 0);

    return nRet;
}

int MvCamera::GetGain(float *returnGain) // 获取增益
{
    int nRet = GetFloatValue("Gain", &m_dGainEdit);

    *returnGain = m_dGainEdit;
    return nRet;
}

int MvCamera::SetGain(float tobeset_Gain) // 设置增益
{
    if (tobeset_Gain < 0)
    {
        return MV_E_PARAMETER;
    }

    m_dGainEdit = tobeset_Gain;
//    SetEnumValue("GainAuto", 0);

    return SetFloatValue("Gain", m_dGainEdit);
}

int MvCamera::SetGainAuto() // 设置连续自动增益
{
    int nRet = SetEnumValue("GainAuto", 2);

    return nRet;
}

int MvCamera::CloseGainAuto()
{
    int nRet = SetEnumValue("GainAuto", 0);

    return nRet;
}

int MvCamera::GetResultingFrameRate(float *returnFrameRate) // 获取帧率
{
    int nRet = GetFloatValue("ResultingFrameRate", &m_dFrameRateEdit);

    *returnFrameRate = m_dFrameRateEdit;
    return nRet;
}

int MvCamera::SetResultingFrameRate(float tobeset_frameRate)
{
    if (tobeset_frameRate < 0)
    {
        return MV_E_PARAMETER;
    }

    m_dFrameRateEdit = tobeset_frameRate;

    return SetFloatValue("ResultingFrameRate", (float)m_dFrameRateEdit);
}
int MvCamera::GetAcquisitionFrameRate(float *returnFrameRate)
{
    int nRet = GetFloatValue("AcquisitionFrameRate", &m_lFrameRateEdit);

    *returnFrameRate = m_lFrameRateEdit;
    return nRet;
}

int MvCamera::SetAcquisitionFrameRate(float tobeset_frameRate)
{
    if (tobeset_frameRate < 0)
    {
        return MV_E_PARAMETER;
    }

    m_lFrameRateEdit = tobeset_frameRate;

    return SetFloatValue("AcquisitionFrameRate", (float)m_lFrameRateEdit);
}

int MvCamera::GetIntValue(IN const char *strKey, OUT unsigned int *pnValue) // 获取Int型参数
{
    if (NULL == strKey || NULL == pnValue)
    {
        return MV_E_PARAMETER;
    }

    MVCC_INTVALUE stParam; // 先定义类型
    memset(&stParam, 0, sizeof(MVCC_INTVALUE));
    int nRet = MV_CC_GetIntValue(m_DeviceHandle, strKey, &stParam);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    *pnValue = stParam.nCurValue; // ncurvalue

    return MV_OK;
}

int MvCamera::SetIntValue(IN const char *strKey, IN unsigned int nValue) // 设置Int型参数
{
    if (NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetIntValue(m_DeviceHandle, strKey, nValue);
}

int MvCamera::GetFloatValue(IN const char *strKey, OUT float *pfValue) // 获取Float型参数
{
    if (NULL == strKey || NULL == pfValue)
    {
        return MV_E_PARAMETER;
    }

    MVCC_FLOATVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_FLOATVALUE));
    int nRet = MV_CC_GetFloatValue(m_DeviceHandle, strKey, &stParam);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    *pfValue = stParam.fCurValue;

    return MV_OK;
}

int MvCamera::SetFloatValue(IN const char *strKey, IN float fValue) // 设置Float型参数
{
    if (NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetFloatValue(m_DeviceHandle, strKey, fValue);
}

int MvCamera::GetEnumValue(IN const char *strKey, OUT unsigned int *pnValue) // 获取Enum型参数
{
    if (NULL == strKey || NULL == pnValue)
    {
        return MV_E_PARAMETER;
    }

    MVCC_ENUMVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_ENUMVALUE));
    int nRet = MV_CC_GetEnumValue(m_DeviceHandle, strKey, &stParam);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    *pnValue = stParam.nCurValue;

    return MV_OK;
}

int MvCamera::SetEnumValue(IN const char *strKey, IN unsigned int nValue) // 设置Enum型参数
{
    if (NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetEnumValue(m_DeviceHandle, strKey, nValue);
}

int MvCamera::GetBoolValue(IN const char *strKey, OUT bool *pbValue) // 获取Bool型参数
{
    if (NULL == strKey || NULL == pbValue)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_GetBoolValue(m_DeviceHandle, strKey, pbValue);
}

int MvCamera::SetBoolValue(IN const char *strKey, IN bool bValue) // 设置Bool型参数
{
    if (NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetBoolValue(m_DeviceHandle, strKey, bValue);
}

int MvCamera::GetStringValue(IN const char *strKey, IN OUT char *strValue, IN unsigned int nSize) // 获取String型参数
{
    if (NULL == strKey || NULL == strValue)
    {
        return MV_E_PARAMETER;
    }

    MVCC_STRINGVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_STRINGVALUE));
    int nRet = MV_CC_GetStringValue(m_DeviceHandle, strKey, &stParam);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    strcpy_s(strValue, nSize, stParam.chCurValue);

    return MV_OK;
}

int MvCamera::SetStringValue(IN const char *strKey, IN const char *strValue) // 设置String型参数
{
    if (NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetStringValue(m_DeviceHandle, strKey, strValue);
}

int MvCamera::GetImageBuffer(MV_FRAME_OUT *pFrame, int nMsec) // 主动获取一帧图像数据
{
    return MV_CC_GetImageBuffer(m_DeviceHandle, pFrame, nMsec);
}

int MvCamera::FreeImageBuffer(MV_FRAME_OUT *pFrame) // 释放图像缓存
{
    return MV_CC_FreeImageBuffer(m_DeviceHandle, pFrame);
}

int MvCamera::GetImageFOrBGR(unsigned char *pData,unsigned int nDataSize,MV_FRAME_OUT_INFO_EX *pstFrameInfo,int nMsec)
{
    return MV_CC_GetImageForBGR(m_DeviceHandle,pData,nDataSize,pstFrameInfo,nMsec);
}

int MvCamera::DisplayOneFrame(MV_DISPLAY_FRAME_INFO *pDisplayInfo)
{
    return MV_CC_DisplayOneFrame(m_DeviceHandle, pDisplayInfo);
}


int MvCamera::ConvertPixelTypeToGray(MV_FRAME_OUT *pFrame,cv::Mat& image)
// 定义目标像素格式为灰度图像
{
    MV_CC_PIXEL_CONVERT_PARAM convertParam;
    convertParam.nWidth = pFrame->stFrameInfo.nWidth;
    convertParam.nHeight = pFrame->stFrameInfo.nHeight;
    convertParam.enSrcPixelType = PixelType_Gvsp_BayerRG8;
    convertParam.enDstPixelType = PixelType_Gvsp_Mono8;

    // 转换为灰度图像
    unsigned char* pData = pFrame->pBufAddr;
    int nDataSize = pFrame->stFrameInfo.nFrameLen;
    unsigned char* pConvertBuf = new unsigned char[nDataSize];
    convertParam.pSrcData = pData;
    convertParam.pDstBuffer = pConvertBuf;
    int ret = MV_CC_ConvertPixelType(this->m_DeviceHandle, &convertParam);

    // 显示灰度图像
    cv::Mat grayImg(pFrame->stFrameInfo.nHeight, pFrame->stFrameInfo.nWidth, CV_8UC1, pConvertBuf);
    grayImg.copyTo(image);
    free(pConvertBuf);  //防止内存泄漏
    return ret;
}

//question:转彩色时采集一段时间后会报opencv内存不足错误
int MvCamera::ConvertPixelTypeToRGB(MV_FRAME_OUT *pFrame,cv::Mat& image)
{
    // 定义目标像素格式为灰度图像
    MV_CC_PIXEL_CONVERT_PARAM convertParam;
    convertParam.nWidth = pFrame->stFrameInfo.nWidth;
    convertParam.nHeight = pFrame->stFrameInfo.nHeight;
    convertParam.enSrcPixelType = PixelType_Gvsp_BayerRG8;
    convertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed;

    // 转换为RGB图像
    unsigned char* pData = pFrame->pBufAddr;
    int nDataSize = pFrame->stFrameInfo.nFrameLen;
    unsigned char* pConvertBuf = new unsigned char[nDataSize * 3];
    convertParam.pSrcData = pData;
    convertParam.pDstBuffer = pConvertBuf;
    int ret = MV_CC_ConvertPixelType(this->m_DeviceHandle, &convertParam);

    // 显示灰度图像
    cv::Mat grayImg(pFrame->stFrameInfo.nHeight, pFrame->stFrameInfo.nWidth, CV_8UC3, pConvertBuf);
    grayImg.copyTo(image);
    free(pConvertBuf);     //防止内存泄漏
    return ret;
}

