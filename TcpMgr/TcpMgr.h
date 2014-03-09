#ifndef TCPMGR_H
#define TCPMGR_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class TcpMgr : public QObject
{
    Q_OBJECT
public:
    explicit TcpMgr(QObject *parent = 0);
    ~ TcpMgr();
    void setSocketConnection(QString ip,int port);
    void send(QString msg);
signals:
    void socketConnected();
    void socketError(QString err);
    void serverError(QString err);
private slots:
    void socketError();
private:
    QTcpServer * m_pTcpServer;
    QTcpSocket * m_pTcpSocket;

};

#endif // TCPMGR_H
