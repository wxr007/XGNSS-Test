#pragma once

#include <QWidget>
#include "ui_NtripClient.h"
#include <QJsonObject>

class NtripClient : public QWidget
{
	Q_OBJECT

public:
	NtripClient(QWidget *parent = nullptr);
	~NtripClient();
	void loadConfig(QJsonObject& json);
	void saveConfig(QJsonObject& json);
public slots:
	void onConnectClicked();
	void onEnable(bool);
	void onDataSize(const int len);
	void onSaveFileChecked(bool checked);
private:
	Ui::NtripClientClass ui;
	int m_DataSize;
};
