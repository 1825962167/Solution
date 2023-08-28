#include "udpsocketthread.h"

udpSocketThread::udpSocketThread(QObject *parent):QThread(parent)
{
    this->udpSocket = new QUdpSocket(this);
}

bool udpSocketThread::bindLocalPort()
{
    if(!udpSocket->bind(QHostAddress(this->localIP),port))
    {
        qDebug() << "[udpSocket] Failed to bind local port:" << port;
        return false;
    }
    qDebug() << "[udpSocket]" << this->udpSocket->localAddress().toString()<<" port:"<<this->udpSocket->localPort();
    return true;
}

void udpSocketThread::writeDatagram(QByteArray sendData)
{
    udpSocket->writeDatagram(sendData, QHostAddress(this->mcuIP), this->mcuport);
}
void udpSocketThread::run()
{
    while(true)
    {
        float pre = distance;
        if(udpSocket->hasPendingDatagrams())
        {
            QByteArray data;
            data.resize(udpSocket->pendingDatagramSize());
            udpSocket->readDatagram(data.data(), data.size());
            this->distance = *reinterpret_cast<float*>(data.data()+4);
            data.clear();
            if(this->distance != pre){
                this->isChange = true;
            }
            else{
                this->isChange = false;
            }
//            qDebug()<<distance<<" "<<isChange;
        }

    }
    msleep(10);
}
