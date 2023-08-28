#include "mytcpserver.h"

MyTcpServer::MyTcpServer(QObject *parent)
    : QTcpServer(parent), m_socket(nullptr)
{
    // 监听指定端口
    if (listen(QHostAddress(this->localIP), port))
    {
        qDebug() << "[server] TcpServer start to listen: " << serverAddress().toString() << ":" << serverPort();
    }
    else
    {
        qDebug() << "[server] Failed to start server! error:"<<errorString();
    }

}

void MyTcpServer::readData()
{qDebug()<<"1 ";
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState && m_socket->bytesAvailable() > 0) {
        this->data = m_socket->readAll();
        qDebug() << "[server] Received data: " << QString::fromUtf8(data.toHex().toUpper());
    }
}

void MyTcpServer::sendData(QTcpSocket *socket, const QString &data)
{
    if (socket && socket->state() == QTcpSocket::ConnectedState) {
        socket->write(data.toUtf8());
    }
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
        // 如果已经有一个客户端连接了，则拒绝新的连接
        QTcpSocket *newSocket = new QTcpSocket(this);
        newSocket->setSocketDescriptor(socketDescriptor);
        newSocket->disconnectFromHost();
        newSocket->deleteLater();
        return;
    }
    // 接受新的连接
    QTcpSocket *socket = new QTcpSocket(this);
    if (!socket->setSocketDescriptor(socketDescriptor))
    {
        delete socket;
        return;
    }
    QHostAddress clientAddress = socket->peerAddress();
    // 如果连接的IP地址与允许连接的IP地址不同，关闭连接
    if (clientAddress != QHostAddress(this->mcuIP))
    {
        socket->close();
        delete socket;
        return;
    }
    this->m_socket = socket;
    qDebug() << "[server] New connection from: " << m_socket->peerAddress().toString();
    connectReadData();
}

void MyTcpServer::connectReadData()
{
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState)
    {
        connect(this->m_socket,SIGNAL(readyRead()),this,SLOT(readData()));
    }

}

void MyTcpServer::disconnectReadData()
{
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState)
    {
        disconnect(this->m_socket,SIGNAL(readyRead()),this,SLOT(readData()));
    }
}
