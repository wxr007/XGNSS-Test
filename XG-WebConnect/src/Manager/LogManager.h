#pragma once

#include <QObject>
#include <QFile>
#include <QDir>
#include <mutex>

enum MsgLevel {
	MSG_INFO,
	MSG_WARNING,
	MSG_ERROR,
};

class LogManager : public QObject
{
	Q_OBJECT

private:
	LogManager(QObject *parent);
	~LogManager();
public:
	static LogManager* getInstance();
	static void createInstance();
private:
	static LogManager* m_instance;
	static std::once_flag m_flag;

public:
	void setLogRoot(QString logPath);
	QString getLogRoot();
	QDir makePath();
	QString getLogPath();
private:
	QString m_logRootPath;
signals:
	void sgnShowMsg(int level,const QString);
};
