#pragma once

#include <QObject>
#include "Base/WebSocketServerBase.h"

class WebSocketServerWorker  : public WebSocketServerBase
{
	Q_OBJECT

public:
	WebSocketServerWorker(QObject *parent);
	~WebSocketServerWorker();
public slots:
	void onOpen();
	void onClose();
	void onWrite(const QByteArray data);
signals:
	void sgnDataSize(const int);
};

class WebSocketServerThread : public WorkerThread
{
	Q_OBJECT
public:
	WebSocketServerThread(QObject* parent = nullptr);
	~WebSocketServerThread();
	void setPort(int nPort);
signals:
	void sgnDataSize(const int);
};