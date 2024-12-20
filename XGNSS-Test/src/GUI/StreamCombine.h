#pragma once

#include <QWidget>
#include "ui_StreamCombine.h"
#include "SerialPortWidget.h"
#include "TcpClientWidget.h"
#include "NtripClient.h"

class StreamCombine : public QWidget
{
	Q_OBJECT

public:
	StreamCombine(QWidget *parent = nullptr);
	~StreamCombine();
	void loadConfig(QJsonObject& json);
	void saveConfig(QJsonObject& json);
private:
	Ui::StreamCombineClass ui;
public:
	SerialportWidget* serialPortWidget;
	TcpClientWidget* tcpClientWidget;
	NtripClient* ntripClient;
};
