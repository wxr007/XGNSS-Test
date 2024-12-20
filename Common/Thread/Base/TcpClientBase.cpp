#include "TcpClientBase.h"
#include <QThread>
#include "LogManager.h"

TcpClientBase::TcpClientBase(QObject* parent, DataParser* dataparser)
	: WorkerBase(parent, dataparser)
	, m_isUserOpen(false)
	, m_isReady(false)
	, m_nPort(0)
{
	m_QTcpSocket = new QTcpSocket(this);
	m_reconnect_timer = new QTimer(this);
	m_reconnect_timer->setSingleShot(true);
	m_reconnect_timer->stop();
	m_reconnect_timer->setInterval(5000);
	connect(m_QTcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(m_QTcpSocket, SIGNAL(readyRead()), this, SLOT(onReadReady()));
	connect(m_QTcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(m_QTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError()));
	connect(m_reconnect_timer, SIGNAL(timeout()), this, SLOT(doConnect()));
}

TcpClientBase::~TcpClientBase()
{
	onClose();
	m_reconnect_timer->stop();
	delete m_QTcpSocket; m_QTcpSocket = NULL;
	delete m_reconnect_timer; m_reconnect_timer = NULL;
}

void TcpClientBase::onOpen()
{
	if (!m_isUserOpen) {
		m_isUserOpen = true;
		m_isReady = false;
		doConnect();
		qDebug() << "TcpClient::open: " << "ThreadId: " << QThread::currentThreadId();
	}
}

void TcpClientBase::onClose()
{
	m_isUserOpen = false;
	m_reconnect_timer->stop();
	doDisconnect();
	WorkerBase::onClose();
}

void TcpClientBase::onWrite(const QByteArray data)
{
	qDebug() << "TcpClient::onWrite: " << "ThreadId: " << QThread::currentThreadId();
	if (m_QTcpSocket->isOpen() && m_isReady) {
		m_QTcpSocket->write(data);
	}
}

void TcpClientBase::doConnect()
{
	m_QTcpSocket->connectToHost(m_strIp, m_nPort);
}

void TcpClientBase::doDisconnect()
{
	if (m_QTcpSocket->isOpen()) {
		m_QTcpSocket->disconnectFromHost();
		m_QTcpSocket->close();
	}
}

void TcpClientBase::onConnected()
{
	qDebug() << "TcpClient::onConnected: " << "ThreadId: " << QThread::currentThreadId();
	emit LogManager::getInstance()->sgnShowMsg(MSG_INFO, "$INFO:" + QString::asprintf("TcpClient connected %s:%d", qPrintable(m_strIp), m_nPort));
	m_isReady = true;
	//emit sgnEnable(false);
}

void TcpClientBase::onDisconnected()
{
	qDebug("TcpClient disconnected %s:%d", qPrintable(m_strIp), m_nPort);
	emit LogManager::getInstance()->sgnShowMsg(MSG_WARNING, "$WARNNING:" + QString::asprintf("TcpClient disconnected %s:%d", qPrintable(m_strIp), m_nPort));
	if (m_isUserOpen) {
		qDebug("TcpClient reconnect %s:%d ", qPrintable(m_strIp), m_nPort);
		if (!m_reconnect_timer->isActive()) {
			emit LogManager::getInstance()->sgnShowMsg(MSG_INFO, "$INFO:" + QString::asprintf("TcpClient reconnect %s:%d", qPrintable(m_strIp), m_nPort));
			m_reconnect_timer->start();
		}
	}
	else {
		//emit sgnEnable(true);
	}
	m_isReady = false;
}

void TcpClientBase::onReadReady()
{
	//qDebug() << "TcpClient::onReadReady: " << "ThreadId: " << QThread::currentThreadId();
	QByteArray byteArray = m_QTcpSocket->readAll();
	//处理数据
	processData(byteArray);
}

void TcpClientBase::onSocketError()
{
	qDebug() << "Error:" << m_QTcpSocket->errorString() << "\n";
	emit LogManager::getInstance()->sgnShowMsg(MSG_ERROR, "$Error:" + m_QTcpSocket->errorString());
	if (m_isUserOpen) {
		doDisconnect();
		if (!m_reconnect_timer->isActive()) {
			emit LogManager::getInstance()->sgnShowMsg(MSG_INFO, "$INFO:" + QString::asprintf("TcpClient reconnect %s:%d", qPrintable(m_strIp), m_nPort));
			m_reconnect_timer->start();
		}
	}
	else {
		onClose();
	}
}

void TcpClientBase::processData(QByteArray& byteArray)
{
}