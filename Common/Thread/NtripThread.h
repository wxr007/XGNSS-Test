#pragma once

#include <QObject>
#include "Base/TcpClientBase.h"

class NtripWorker : public TcpClientBase 
{
	Q_OBJECT
public:
	NtripWorker(QObject* parent = nullptr);
	~NtripWorker();
	void setNtripParams(QString sIp, int nPort, QString sMountPoint, QString sUserName, QString sPassword);
	void sendNtripHead();
public slots:
	void onOpen();
	void onClose();
protected slots:
	void onConnected();
protected:
	virtual void processData(QByteArray& byteArray);
private:
	QString m_sMountPoint;
	QString m_sUserName;
	QString m_sPassword;
signals:
	void sgnStream(const QByteArray);
	void sgnDataSize(const int);
};

class NtripThread : public WorkerThread
{
	Q_OBJECT
public:
	NtripThread(QObject* parent = nullptr);
	~NtripThread();
	void setNtripParams(QString sIp, int nPort, QString sMountPoint, QString sUserName, QString sPassword);
signals:
	void sgnStream(const QByteArray);
	void sgnDataSize(const int);
};