#ifndef GETIMAGETHREAD_H
#define GETIMAGETHREAD_H

#include "MvCamera.h"
#include <QThread>
#include <QMutex>

class GetImageThread
    : public QThread
{
    Q_OBJECT

public:
    GetImageThread(QObject *parent = nullptr);

    GetImageThread(MvCamera **pnt2MvCamera);

    ~GetImageThread();

public slots:
    void setGetImageThreadFlag(bool grabFlag);
    bool getGetImageThreadFlag();

signals:
    void newFrameOut(MV_FRAME_OUT newFrameOut);

protected:
    void run() override;

private:
    MvCamera *m_MvCamera;
    MV_FRAME_OUT m_stFrameOut;
    MV_FRAME_OUT *m_pntFrameOut;

    QMutex m_mutex;

public:
    bool m_ThreadEnabled;
};

#endif // MVGRABTHREAD_H
