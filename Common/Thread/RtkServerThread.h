#pragma once

#include <QObject>
#include "Base/TcpServerBase.h"
#include "DataParser.h"

/// <summary>
/// 用来接收ntrip转发的数据
/// </summary>

class RtkServerWorker  : public TcpServerBase
{
	Q_OBJECT
public:
	RtkServerWorker(QObject *parent);
	~RtkServerWorker();
public slots:
	void onOpen();
	void onClose();
protected:
	virtual void processData(QString ip, int port, QByteArray& byteArray);
signals:
	void sgnStream(const QByteArray);
	void sgnDataSize(const int);
};

class RtkServerThread : public WorkerThread
{
	Q_OBJECT
public:
	RtkServerThread(QObject* parent = nullptr);
	~RtkServerThread();
	void setPort(int nPort);
signals:
	void sgnStream(const QByteArray);
	void sgnDataSize(const int);
};