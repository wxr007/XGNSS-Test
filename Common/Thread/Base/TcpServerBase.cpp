#include "TcpServerBase.h"
#include "LogManager.h"

TcpServerBase::TcpServerBase(QObject *parent, DataParser* dataparser)
	: WorkerBase(parent, dataparser)
	, m_nPort(0)
{
	m_Server = new QTcpServer(this);
	m_Clients.clear();

	connect(m_Server, SIGNAL(newConnection()), this, SLOT(onAccept()));
}

TcpServerBase::~TcpServerBase()
{}

void TcpServerBase::setPort(int port)
{
    m_nPort = port;
}

void TcpServerBase::onOpen()
{
    if(m_nPort) m_Server->listen(QHostAddress::Any, m_nPort);
}

void TcpServerBase::onClose()
{
    closeClients();
	m_Server->close();
    WorkerBase::onClose();
}

void TcpServerBase::onWrite(const QByteArray data)
{
    foreach(QTcpSocket * socket, m_Clients) {
        socket->write(data);
    }
}

void TcpServerBase::onAccept()
{
    qDebug() << "TcpServerBase::Accept: " << "ThreadId: " << QThread::currentThreadId();
    QTcpSocket* socket = m_Server->nextPendingConnection();
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadReady()));
    //连接后加入链表
    m_Clients.append(socket);
    //IP端口信息
    QString ip = socket->peerAddress().toString();
    ip = ip.replace("::ffff:", "");
    int port = socket->peerPort();
    //处理新链接
    processAccept(ip, port);
}

void TcpServerBase::onDisconnected()
{
    qDebug() << "TcpServerBase::onDisconnected: " << "ThreadId: " << QThread::currentThreadId();
    QTcpSocket* socket = (QTcpSocket*)sender();
    //IP端口信息
    QString ip = socket->peerAddress().toString();
    ip = ip.replace("::ffff:", "");
    int port = socket->peerPort();
    //处理断开链接
    processDisconnected(ip, port);
    //断开连接后从链表中移除
    m_Clients.removeOne(socket);
    socket->deleteLater();
}

void TcpServerBase::onReadReady()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    QByteArray data = socket->readAll();
    if (data.length() <= 0) {
        return;
    }
    //IP端口信息
    QString ip = socket->peerAddress().toString();
    ip = ip.replace("::ffff:", "");
    int port = socket->peerPort();
    //处理数据
    processData(ip, port, data);
}

void TcpServerBase::onSocketError()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    qDebug() << "TcpServerBase::onSocketError: " << "ThreadId: " << QThread::currentThreadId() << socket->errorString();
    //IP端口信息
    QString ip = socket->peerAddress().toString();
    ip = ip.replace("::ffff:", "");
    int port = socket->peerPort();
    //处理错误
    processError(ip, port, socket->errorString());
}

void TcpServerBase::processAccept(QString ip, int port)
{
    emit LogManager::getInstance()->sgnShowMsg(MSG_INFO, "$INFO:" + QString::asprintf("TcpServer accept %s:%d", qPrintable(ip), port));
}

void TcpServerBase::processDisconnected(QString ip, int port)
{
    emit LogManager::getInstance()->sgnShowMsg(MSG_WARNING, "$WARNNING:" + QString::asprintf("TcpServer disconnected %s:%d", qPrintable(ip), port));
}

void TcpServerBase::processData(QString ip, int port, QByteArray& byteArray)
{

}

void TcpServerBase::processError(QString ip, int port, QString error)
{
    emit LogManager::getInstance()->sgnShowMsg(MSG_ERROR, "$ERROR:" + QString::asprintf("TcpServer error %s:%d %s", qPrintable(ip), port, qPrintable(error)));
}

void TcpServerBase::closeClients()
{
    foreach(QTcpSocket * socket, m_Clients) {
        socket->abort();
        delete socket;
    }
    m_Clients.clear();
}
