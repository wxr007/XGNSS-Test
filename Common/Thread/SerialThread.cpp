#include "SerialThread.h"
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include "DataCache.h"
#include "LogManager.h"

SerialWorker::SerialWorker(QObject* parent, DataParser* dataparser)
    : WorkerBase(parent, dataparser)
{
	m_FileNameFlag = "user";
	m_QextSerialPort = new QextSerialPort(QextSerialPort::EventDriven,this);
	connect(m_QextSerialPort, SIGNAL(readyRead()), this, SLOT(onReadReady()), Qt::DirectConnection);
	qDebug() << "SerialWorker: " << "ThreadId: " << QThread::currentThreadId();
}

SerialWorker::~SerialWorker()
{
	onClose();
	delete m_QextSerialPort; m_QextSerialPort = NULL;
}

void SerialWorker::setSerialPortParams(QString sName, int nBaudRate, int nDataBits, int nParity, int nStopBits)
{
	m_QextSerialPort->setPortName(sName);
	m_QextSerialPort->flush();
	m_QextSerialPort->setBaudRate((BaudRateType)nBaudRate);

	switch (nDataBits)
	{
	case 5: m_QextSerialPort->setDataBits(DATA_5); break;
	case 6: m_QextSerialPort->setDataBits(DATA_6); break;
	case 7: m_QextSerialPort->setDataBits(DATA_7); break;
	case 8: m_QextSerialPort->setDataBits(DATA_8); break;
	default: break;
	}
	switch (nParity)
	{
	case 0: m_QextSerialPort->setParity(PAR_NONE); break;
	case 1: m_QextSerialPort->setParity(PAR_ODD); break;
	case 2: m_QextSerialPort->setParity(PAR_EVEN); break;
	case 3: m_QextSerialPort->setParity(PAR_MARK); break;
	case 4: m_QextSerialPort->setParity(PAR_SPACE); break;
	default: break;
	}
	switch (nStopBits)
	{
	case 0:m_QextSerialPort->setStopBits(STOP_1); break;
	case 1:m_QextSerialPort->setStopBits(STOP_1_5); break;
	case 2:m_QextSerialPort->setStopBits(STOP_2); break;
	default: break;
	}
	m_QextSerialPort->setFlowControl(FLOW_OFF);
	m_QextSerialPort->setTimeout(10);
}

void SerialWorker::setDataFormat(int index)
{
	m_DataParser->setDataFormat(index);
}

void SerialWorker::onOpen()
{
	m_DataParser->init();
	openDataFile();
	bool isOpen = m_QextSerialPort->open(QIODevice::ReadWrite);
	if (!isOpen) {
		emit sgnOpenFailed();
	}
	qDebug() << "SerialWorker::start: " << "ThreadId: " << QThread::currentThreadId();
}

void SerialWorker::onClose()
{
	m_QextSerialPort->close();
	closeDataFile();
	WorkerBase::onClose();
}

void SerialWorker::onWrite(const QByteArray data)
{
	//qDebug() << "TcpClientWorker::onWrite: " << "ThreadId: " << QThread::currentThreadId();
	if (m_QextSerialPort->isOpen() && m_QextSerialPort->isWritable()) {
		m_QextSerialPort->write(data);
	}
}

void SerialWorker::onReadReady()
{
	//qDebug() << "SerialWorker::onReadReady: " << "ThreadId: " << QThread::currentThreadId();
	QByteArray buffer = m_QextSerialPort->readAll();
	//写文件
	writeDataLog(buffer);
	//解析数据发送到外部显示
	m_DataParser->parseData(buffer);
	//显示接收字节
	emit sgnDataSize(buffer.size());
}

SerialThread::SerialThread(QObject* parent, DataParser* dataparser)
	:WorkerThread(parent, new SerialWorker(NULL, dataparser))
{
	connect(m_Worker, SIGNAL(sgnDataSize(const int)), this, SIGNAL(sgnDataSize(const int)));
}

SerialThread::~SerialThread()
{
}

void SerialThread::setSerialPortParams(QString sName, int nBaudRate, int nDataBits, int nParity, int nStopBits)
{
	((SerialWorker*)m_Worker)->setSerialPortParams(sName, nBaudRate, nDataBits, nParity, nStopBits);
}

void SerialThread::setDataFormat(int index)
{
	((SerialWorker*)m_Worker)->setDataFormat(index);
}

