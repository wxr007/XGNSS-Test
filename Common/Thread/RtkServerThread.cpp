#include "RtkServerThread.h"
#include "LogManager.h"

RtkServerWorker::RtkServerWorker(QObject *parent)
	: TcpServerBase(parent,NULL)
{
	m_FileNameFlag = "base";
	qDebug() << "RtkServerWorker: " << "ThreadId: " << QThread::currentThreadId();
}

RtkServerWorker::~RtkServerWorker()
{
	onClose();
}

void RtkServerWorker::onOpen()
{
	TcpServerBase::onOpen();
	openDataFile();
	qDebug() << "RtkServerWorker::open: " << "ThreadId: " << QThread::currentThreadId();
	emit LogManager::getInstance()->sgnShowMsg(MSG_INFO, "$INFO:" + QString::asprintf("RtkServer listen on %d", m_nPort));
}

void RtkServerWorker::onClose()
{
	closeDataFile();
	qDebug() << "RtkServerWorker::close: " << "ThreadId: " << QThread::currentThreadId();
	emit LogManager::getInstance()->sgnShowMsg(MSG_INFO, "$INFO:" + QString::asprintf("RtkServer closed on %d", m_nPort));
	TcpServerBase::onClose();
}

void RtkServerWorker::processData(QString ip, int port, QByteArray & byteArray)
{
	emit sgnStream(byteArray);
	emit sgnDataSize(byteArray.size());
}

RtkServerThread::RtkServerThread(QObject* parent)
	:WorkerThread(parent, new RtkServerWorker(NULL))
{
	connect(m_Worker, SIGNAL(sgnStream(const QByteArray)), this, SIGNAL(sgnStream(const QByteArray)));
	connect(m_Worker, SIGNAL(sgnDataSize(const int)), this, SIGNAL(sgnDataSize(const int)));
}

RtkServerThread::~RtkServerThread()
{
}

void RtkServerThread::setPort(int nPort)
{
	((RtkServerWorker*)m_Worker)->setPort(nPort);
}
