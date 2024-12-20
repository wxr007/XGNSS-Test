#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include "WorkerBase.h"

class TcpClientBase  : public WorkerBase
{
	Q_OBJECT

public:
	TcpClientBase(QObject *parent = nullptr, DataParser* dataparser = nullptr);
	virtual ~TcpClientBase();
public:
	virtual void onOpen();
	virtual void onClose();
public slots:
	void onWrite(const QByteArray data);
protected slots:
	void doConnect();
	void doDisconnect();
	virtual void onConnected();
	void onDisconnected();
	void onReadReady();
	void onSocketError();
protected:
	virtual void processData(QByteArray& byteArray);
protected:
	QString m_strIp;
	int m_nPort;
	bool m_isUserOpen;	//判断是否用户主动关闭
	bool m_isReady;
private:
	QTcpSocket* m_QTcpSocket;
	QTimer* m_reconnect_timer;
};
