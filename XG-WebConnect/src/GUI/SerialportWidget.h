#pragma once

#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo> 
#include "ui_SerialportWidget.h"
#include <QJsonObject>

class SerialportWidget : public QWidget
{
	Q_OBJECT

public:
	SerialportWidget(QWidget *parent = nullptr);
	~SerialportWidget();
	void searchPorts();
	void loadConfig(QJsonObject& json);
	void saveConfig(QJsonObject& json);
public slots:
	void onRefreshClicked();
	void onStartClicked();
	void onEnable(bool enable);
	void onOpenFailed();
	void onOtherStreamEnable(bool enable);
	void onDataSize(const int len);
private:
	Ui::SerialportWidgetClass ui;
	uint64_t m_DataSize;
};
