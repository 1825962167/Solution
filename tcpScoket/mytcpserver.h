#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>

class MyTcpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit MyTcpServer(QObject *parent = nullptr);

    void sendData(QTcpSocket *socket, const QString &data); // 给指定的客户端发送数据

    void connectReadData();

    void disconnectReadData();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void readData(QByteArray data);

public slots:
    void readData(); // 读取客户端数据

private:
    QTcpSocket *m_socket; // 客户端socket指针
    QString mcuIP = "192.168.69.160";
    QString localIP = "192.168.69.11";
    int port = 10006;

public:
    QByteArray data;
};

#endif // MYTCPSERVER_H
