#pragma once

#include <QWidget>
#include "ui_WebSocketUI.h"

class WebSocketUI : public QWidget
{
	Q_OBJECT

public:
	WebSocketUI(QWidget *parent = nullptr);
	~WebSocketUI();
public slots:
	void onStartClicked();
	void onEnable(bool enable);
	void onDataSize(const int len);
private:
	Ui::WebSocketUIClass ui;
	uint64_t m_DataSize;
};
