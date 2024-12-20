#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include "WorkerBase.h"

class TcpServerBase  : public WorkerBase
{
	Q_OBJECT

public:
	TcpServerBase(QObject *parent, DataParser* dataparser = nullptr);
	virtual ~TcpServerBase();
public:
	void setPort(int port);
	virtual void onOpen();
	virtual void onClose();
public slots:
	void onWrite(const QByteArray data);
protected slots:
	void onAccept();
	void onDisconnected();
	void onReadReady();
	void onSocketError();
protected:
	//发送到上层处理
	virtual void processAccept(QString ip,int port);
	virtual void processDisconnected(QString ip,int port);
	virtual void processData(QString ip,int port,QByteArray& byteArray);
	virtual void processError(QString ip,int port, QString error);
	void closeClients();
protected:
	int m_nPort;
private:
	QTcpServer* m_Server;
	QList<QTcpSocket*> m_Clients;
};
