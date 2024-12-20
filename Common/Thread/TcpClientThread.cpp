#include "TcpClientThread.h"
#include <QDebug>
#include <QDateTime>
#include "DataCache.h"
#include "LogManager.h"
#include "FileManager.h"

TcpClientWorker::TcpClientWorker(QObject *parent, DataParser* dataparser)
	: TcpClientBase(parent, dataparser)
{
	m_FileNameFlag = "user";
	qDebug() << "TcpClientWorker: " << "ThreadId: " << QThread::currentThreadId();
}

TcpClientWorker::~TcpClientWorker()
{
	onClose();
}

void TcpClientWorker::setTcpClientParams(QString sIp, int nPort)
{
	m_strIp = sIp;
	m_nPort = nPort;
}

void TcpClientWorker::setDataFormat(int index)
{
	m_DataParser->setDataFormat(index);
}

void TcpClientWorker::onOpen()
{
	TcpClientBase::onOpen();
	if (m_isUserOpen) {
		m_DataParser->init();
		openDataFile();
		qDebug() << "TcpClientWorker::open: " << "ThreadId: " << QThread::currentThreadId();
	}
}

void TcpClientWorker::onClose()
{
	closeDataFile();
	TcpClientBase::onClose();
}

void TcpClientWorker::processData(QByteArray& byteArray)
{
	//д�ļ�
	writeDataLog(byteArray);
	//�������ݷ��͵��ⲿ��ʾ
	m_DataParser->parseData(byteArray);
	//��ʾ�����ֽ�
	emit sgnDataSize(byteArray.size());
}

TcpClientThread::TcpClientThread(QObject* parent, DataParser* dataparser)
	:WorkerThread(parent, new TcpClientWorker(NULL, dataparser))
{
	connect(m_Worker, SIGNAL(sgnDataSize(const int)), this, SIGNAL(sgnDataSize(const int)));
}

TcpClientThread::~TcpClientThread()
{
}

void TcpClientThread::setTcpClientParams(QString sIp, int nPort)
{
	((TcpClientWorker*)m_Worker)->setTcpClientParams(sIp, nPort);
}

void TcpClientThread::setDataFormat(int index)
{
	((TcpClientWorker*)m_Worker)->setDataFormat(index);
}