#include "WebSocketServerThread.h"

WebSocketServerWorker::WebSocketServerWorker(QObject *parent)
	: WebSocketServerBase(parent, NULL)
{
	m_FileNameFlag = "web";
	qDebug() << "WebSocketServerWorker: " << "ThreadId: " << QThread::currentThreadId();
}

WebSocketServerWorker::~WebSocketServerWorker()
{
	onClose();
}

void WebSocketServerWorker::onOpen()
{
	WebSocketServerBase::onOpen();
	qDebug() << "WebSocketServerWorker::open: " << "ThreadId: " << QThread::currentThreadId();
}

void WebSocketServerWorker::onClose()
{
	qDebug() << "WebSocketServerWorker::close: " << "ThreadId: " << QThread::currentThreadId();
	WebSocketServerBase::onClose();
}

void WebSocketServerWorker::onWrite(const QByteArray data)
{
	WebSocketServerBase::onWrite(data);
	emit sgnDataSize(data.size());
}


WebSocketServerThread::WebSocketServerThread(QObject* parent)
	:WorkerThread(parent, new WebSocketServerWorker(NULL))
{
	//connect(m_Worker, SIGNAL(sgnStream(const QByteArray)), this, SIGNAL(sgnStream(const QByteArray)));
	connect(m_Worker, SIGNAL(sgnDataSize(const int)), this, SIGNAL(sgnDataSize(const int)));
}

WebSocketServerThread::~WebSocketServerThread()
{
}

void WebSocketServerThread::setPort(int nPort)
{
	((WebSocketServerWorker*)m_Worker)->setPort(nPort);
}
