#include "WebSocketServerBase.h"

WebSocketServerBase::WebSocketServerBase(QObject* parent, DataParser* dataparser)
	: WorkerBase(parent, dataparser)
	, m_nPort(0)
{
	web_server = new QWebSocketServer("XGWebServer", QWebSocketServer::NonSecureMode, this);
}

WebSocketServerBase::~WebSocketServerBase()
{
	onClose();
	web_server->deleteLater();
}

void WebSocketServerBase::setPort(int port)
{
	m_nPort = port;
}

void WebSocketServerBase::onOpen()
{
	if (!m_nPort) return;
	if (web_server->listen(QHostAddress::Any, m_nPort)){
		connect(web_server, &QWebSocketServer::newConnection, this, &WebSocketServerBase::onAccept);
		qDebug() << "WebSocket is start, port:" << m_nPort;
	}
}

void WebSocketServerBase::onClose()
{
	disconnect(web_server, &QWebSocketServer::newConnection, this, &WebSocketServerBase::onAccept);
	closeClients();
	web_server->close();
	WorkerBase::onClose();
}

void WebSocketServerBase::onWrite(const QByteArray data)
{
	foreach(QWebSocket * socket, m_Clients) {
		if (socket->isValid())
			socket->sendTextMessage(data);
	}
}

void WebSocketServerBase::onAccept()
{
	qDebug() << "WebSocketServerBase::Accept: " << "ThreadId: " << QThread::currentThreadId();
	QWebSocket* socket = web_server->nextPendingConnection();
	connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	// 处理接收到的消息
	connect(socket, &QWebSocket::textMessageReceived, this, &WebSocketServerBase::onTextReceived);
	//连接后加入链表
	m_Clients.append(socket);
}

void WebSocketServerBase::onDisconnected()
{
	qDebug() << "WebSocketServerBase::onDisconnected: " << "ThreadId: " << QThread::currentThreadId();
	QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
	m_Clients.removeOne(socket);
	socket->deleteLater();
}

void WebSocketServerBase::onTextReceived(const QString& message)
{
	qDebug() << "client received: " << message;
}

void WebSocketServerBase::closeClients()
{
	foreach(QWebSocket* socket, m_Clients) {
		socket->close();
		socket->deleteLater();
	}
	m_Clients.clear();
}
