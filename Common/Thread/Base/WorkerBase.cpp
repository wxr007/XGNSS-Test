#include "WorkerBase.h"
#include <QDateTime>
#include "LogManager.h"
#include "FileManager.h"

WorkerBase::WorkerBase(QObject* parent, DataParser* dataparser)
	: QObject(parent)
{
	m_isLogFile = false;
	m_DataParser = dataparser;
}

WorkerBase::~WorkerBase()
{
	m_DataParser = NULL;
}

void WorkerBase::setLogFile(bool enable)
{
	m_isLogFile = enable;
	if (!m_isLogFile) {
		closeDataFile();
	}
}

void WorkerBase::setFileName(QString sName)
{
	m_FileNameFlag = sName;
}

void WorkerBase::onOpen()
{
}

void WorkerBase::onClose()
{
	emit sgnStop();
}

void WorkerBase::onWrite(const QByteArray data)
{
	qDebug("onWrite\n");
}

void WorkerBase::openDataFile() {
	if (m_isLogFile == false) {
		return;
	}
	if (m_DataFile.isOpen()) {
		return;
	}
	QDir targetDir = LogManager::getInstance()->makePath();
	QDateTime currentTime = QDateTime::currentDateTime();
	QString strTime = currentTime.toString("yyyy-MM-dd_hh-mm-ss");
	QString file_base_path = targetDir.absolutePath() + QDir::separator() + m_FileNameFlag  +"_" + strTime;
	QString file_path = file_base_path + ".dat";
	m_DataFile.setFileName(file_path);
	m_DataFile.open(QIODevice::WriteOnly);
	//解码文件位置设置
	if (m_DataParser) {
		FileManager::Instance().init();
		FileManager::Instance().set_base_file_name(file_base_path.toLocal8Bit().data());
	}
}

void WorkerBase::closeDataFile()
{
	if (m_DataFile.isOpen()) {
		m_DataFile.close();
	}
	//解码文件关闭
	if (m_DataParser) {
		FileManager::Instance().close_all_files();
	}
}

void WorkerBase::writeDataLog(const QByteArray& data) {
	if (m_isLogFile == true && !m_DataFile.isOpen()) {
		openDataFile();
	}
	if (m_DataFile.isOpen())
	{
		m_DataFile.write(data);
	}
}

WorkerThread::WorkerThread(QObject* parent, WorkerBase* worker)
	: QObject(parent)
{
	m_Thread = new QThread(this);
	m_Worker = worker;
	m_Worker->moveToThread(m_Thread);

	connect(this, SIGNAL(sgnOpen()), m_Worker, SLOT(onOpen()), Qt::QueuedConnection);
	connect(this, SIGNAL(sgnClose()), m_Worker, SLOT(onClose()), Qt::QueuedConnection);
	connect(this, SIGNAL(sgnWrite(const QByteArray)), m_Worker, SLOT(onWrite(const QByteArray)));
	connect(m_Worker, SIGNAL(sgnStop()), this, SLOT(onTerminate()), Qt::QueuedConnection);
	connect(m_Worker, SIGNAL(sgnOpenFailed()), this, SIGNAL(sgnOpenFailed()), Qt::QueuedConnection);
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::setLogFile(bool enable)
{
	m_Worker->setLogFile(enable);
}

void WorkerThread::setFileName(QString sName)
{
	m_Worker->setFileName(sName);
}

void WorkerThread::start()
{
	if (m_Thread->isRunning()) {
		return;
	}
	m_Thread->start();
	emit sgnOpen();
	emit sgnEnable(true);
}

void WorkerThread::stop()
{
	//if (!m_Thread->isRunning()) {
	//	return;
	//}
	emit sgnClose();
}

bool WorkerThread::isRunning()
{
	return m_Thread->isRunning();
}

void WorkerThread::onTerminate()
{
	m_Thread->terminate();
	m_Thread->wait();
	emit sgnEnable(false);
}
