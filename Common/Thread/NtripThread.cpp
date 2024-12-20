#include "NtripThread.h"
#include <QDebug>

NtripWorker::NtripWorker(QObject* parent)
	: TcpClientBase(parent, NULL)
{
	m_FileNameFlag = "base";
	qDebug() << "NtripWorker: " << "ThreadId: " << QThread::currentThreadId();
}

NtripWorker::~NtripWorker()
{
	onClose();
}

void NtripWorker::setNtripParams(QString sIp, int nPort, QString sMountPoint, QString sUserName, QString sPassword)
{
	m_strIp = sIp;
	m_nPort = nPort;
	m_sMountPoint = sMountPoint;
	m_sUserName = sUserName;
	m_sPassword = sPassword;
}

void NtripWorker::sendNtripHead()
{
	QString NtripData;
	QString nameAndPassword = m_sUserName + ":" + m_sPassword; ;
	NtripData = QString("GET /%1 HTTP/1.1\r\nUser-Agent: NTRIP IPC Client/0.2\r\nAuthorization: Basic %2\r\n\r\n").arg(m_sMountPoint, nameAndPassword.toLocal8Bit().toBase64());
	onWrite(NtripData.toUtf8());
}

void NtripWorker::onOpen()
{
	TcpClientBase::onOpen();
	if (m_isUserOpen) {
		openDataFile();
		qDebug() << "NtripWorker::open: " << "ThreadId: " << QThread::currentThreadId();
	}
}

void NtripWorker::onClose()
{
	closeDataFile();
	TcpClientBase::onClose();
}

void NtripWorker::onConnected()
{
	TcpClientBase::onConnected();
	sendNtripHead();
}

void NtripWorker::processData(QByteArray& byteArray)
{
	writeDataLog(byteArray);
	emit sgnStream(byteArray);
	emit sgnDataSize(byteArray.size());
}

NtripThread::NtripThread(QObject* parent)
	:WorkerThread(parent, new NtripWorker(NULL))
{
	connect(m_Worker,SIGNAL(sgnStream(const QByteArray)),this,SIGNAL(sgnStream(const QByteArray)));
	connect(m_Worker,SIGNAL(sgnDataSize(const int)),this,SIGNAL(sgnDataSize(const int)));
}

NtripThread::~NtripThread()
{
}

void NtripThread::setNtripParams(QString sIp, int nPort, QString sMountPoint, QString sUserName, QString sPassword)
{
	((NtripWorker*)m_Worker)->setNtripParams(sIp, nPort, sMountPoint, sUserName, sPassword);
}
