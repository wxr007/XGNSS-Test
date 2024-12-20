#pragma once

#include <QWidget>
#include "ui_TcpClientWidget.h"

class TcpClientWidget : public QWidget
{
	Q_OBJECT

public:
	TcpClientWidget(QWidget *parent = nullptr);
	~TcpClientWidget();
	void loadConfig(QJsonObject& json);
	void saveConfig(QJsonObject& json);
public slots:
	void onStartClicked();
	void onEnable(bool enable);
	void onOtherStreamEnable(bool enable);
private:
	Ui::TcpClientWidgetClass ui;
};
