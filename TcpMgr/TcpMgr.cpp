#include "TcpMgr.h"
#include <QDebug>

TcpMgr::TcpMgr(QObject *parent) :
    QObject(parent)
{
    m_pTcpServer = new QTcpServer(this);
    m_pTcpSocket = new QTcpSocket(this);

    connect(m_pTcpSocket, SIGNAL(connected()), this, SIGNAL(socketConnected()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));

}

TcpMgr::~TcpMgr()
{
    m_pTcpServer->deleteLater();
    m_pTcpSocket->deleteLater();
}

void TcpMgr::setSocketConnection(QString ip, int port)
{
    m_pTcpSocket->abort();

    if(!m_pTcpServer->listen(QHostAddress(ip),port))
    {
        emit serverError(m_pTcpServer->errorString());
        return;
    }

    m_pTcpSocket->connectToHost(QHostAddress(ip),port);
}

void TcpMgr::send(QString msg)
{
    if(m_pTcpSocket->state() == QTcpSocket::ConnectedState)
    {
        m_pTcpSocket->write(msg.toLocal8Bit());
    }
    else
    {
        qDebug()<<"socket is not connected!";
    }
}

void TcpMgr::socketError()
{
    emit socketError(m_pTcpSocket->errorString());
}



