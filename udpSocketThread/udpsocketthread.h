#ifndef UDPSOCKETTHREAD_H
#define UDPSOCKETTHREAD_H
#include <QThread>
#include <QUdpSocket>


class udpSocketThread: public QThread
{
    Q_OBJECT

public:
    udpSocketThread(QObject *parent = nullptr);

    void writeDatagram(QByteArray sendData);

    bool bindLocalPort();

protected:
    void run() override;

private:
    QUdpSocket *udpSocket;
    QString localIP = "192.168.69.11";
    QString mcuIP = "192.168.69.160";     //单片机IP地址
    int mcuport = 10006;                  //单片机端口号
    int port = 10006;                     //主机与mcu通信默认端口

public:
    float distance = 0;                   //返回的光栅尺位置
    bool isChange = false;                //光栅尺位置是否改变
};

#endif // UDPSOCKETTHREAD_H
