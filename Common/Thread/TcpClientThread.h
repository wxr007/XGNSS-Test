#pragma once

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QTimer>
#include "Base/TcpClientBase.h"
#include "DataParser.h"

class TcpClientWorker : public TcpClientBase
{
	Q_OBJECT
public:
	explicit TcpClientWorker(QObject *parent = nullptr, DataParser* dataparser = nullptr);
	~TcpClientWorker();
	void setTcpClientParams(QString sIp, int nPort);
	void setDataFormat(int index);
public slots:
	void onOpen();
	void onClose();
protected:
	virtual void processData(QByteArray& byteArray);
signals:
	void sgnDataSize(const int);
};

class TcpClientThread : public WorkerThread
{
	Q_OBJECT
public:
	TcpClientThread(QObject* parent = nullptr, DataParser* dataparser = nullptr);
	~TcpClientThread();
	void setTcpClientParams(QString sIp, int nPort);
	void setDataFormat(int index);
signals:
	void sgnDataSize(const int);
};