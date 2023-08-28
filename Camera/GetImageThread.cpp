#include "GetImageThread.h"
#include <QDebug>

GetImageThread::GetImageThread(QObject *parent)
    : QThread(parent)
{
    m_ThreadEnabled = false;
}

GetImageThread::GetImageThread(MvCamera **pnt2MvCamera)
    : m_MvCamera(*pnt2MvCamera)
{
    m_ThreadEnabled = false;
}

GetImageThread::~GetImageThread(){}


void GetImageThread::run()
{
    int nRet = MV_OK;

    while (m_ThreadEnabled)
    {
//        qDebug() << "mvGrabThread Running";

        nRet = m_MvCamera->GetImageBuffer(&m_stFrameOut, 1000);
        if (nRet == MV_OK)
        {
            // 通过信号-槽机制
//            m_mutex.lock();
            emit newFrameOut(m_stFrameOut);
            m_MvCamera->FreeImageBuffer(&m_stFrameOut);
//            m_mutex.unlock();
        }

        msleep(500);
    }

    return; // 线程主动结束
}

void GetImageThread::setGetImageThreadFlag(bool grabFlag)
{
    m_ThreadEnabled = grabFlag;
}


bool GetImageThread::getGetImageThreadFlag()
{
    return m_ThreadEnabled;
}
