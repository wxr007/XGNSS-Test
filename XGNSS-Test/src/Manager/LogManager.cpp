#include "LogManager.h"
#include <thread>
#include <mutex>
#include <iostream>
#include <QDateTime>

LogManager * LogManager::m_instance = NULL;
std::once_flag      LogManager::m_flag;

LogManager::LogManager(QObject *parent)
	: QObject(parent)
{

}

LogManager::~LogManager()
{
}

LogManager* LogManager::getInstance()
{
	if (m_instance == NULL)
	{
		try
		{
			std::call_once(m_flag, createInstance);
		}
		catch (...)
		{
			std::cout << "CreateInstance error\n";
		}
	}
	return m_instance;
}

void LogManager::createInstance()
{
	m_instance = new(std::nothrow) LogManager(NULL);
	if (NULL == m_instance)
	{
		throw std::exception();
	}
}

void LogManager::setLogRoot(QString logPath)
{
	m_logRootPath = logPath;
	QDir targetDir(logPath);
	if (!targetDir.exists()) {
		emit sgnShowMsg(MSG_WARNING,"Can't find path: " + logPath);
		emit sgnShowMsg(MSG_WARNING,"Please select another path at [Control-> Options-> log path].");
	}
}

QString LogManager::getLogRoot()
{
	return m_logRootPath;
}

QDir LogManager::makePath()
{
	QDateTime currentTime = QDateTime::currentDateTime();
	QString strData = currentTime.toString("yyyy-MM-dd");
	QDir targetDir(m_logRootPath + QDir::separator() + strData);
	if (!targetDir.exists()) {
		targetDir.mkpath(targetDir.absolutePath());
	}
	if (!targetDir.exists()) {
		emit sgnShowMsg(MSG_WARNING, "Can't find path: " + m_logRootPath);
		emit sgnShowMsg(MSG_WARNING, "Please select another path at [Control-> Options-> log path].");
	}
	return targetDir;
}

QString LogManager::getLogPath()
{
	if (m_logRootPath.isEmpty())
	{
		return m_logRootPath;
	}
	QDateTime currentTime = QDateTime::currentDateTime();
	QString strData = currentTime.toString("yyyy-MM-dd");
	QDir targetDir(m_logRootPath + QDir::separator() + strData);
	if (!targetDir.exists()) {
		return m_logRootPath;
	}
	return targetDir.absolutePath();
}
