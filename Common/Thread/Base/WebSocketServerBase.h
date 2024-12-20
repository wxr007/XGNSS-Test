#pragma once

#include <QObject>
#include <QtWebSockets>
#include "WorkerBase.h"

class WebSocketServerBase  : public WorkerBase
{
	Q_OBJECT

public:
	WebSocketServerBase(QObject *parent, DataParser* dataparser = nullptr);
	virtual ~WebSocketServerBase();
public:
	void setPort(int port);
	virtual void onOpen();
	virtual void onClose();
public slots:
	virtual void onWrite(const QByteArray data);
protected slots:
	void onAccept();
	void onDisconnected();
	void onTextReceived(const QString& message);
protected:
	void closeClients();
protected:
	int m_nPort;
	QWebSocketServer* web_server;
	QList<QWebSocket*> m_Clients;
};
