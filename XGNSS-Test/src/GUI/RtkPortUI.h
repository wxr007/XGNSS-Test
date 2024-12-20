#pragma once

#include <QWidget>
#include "ui_RtkPortUI.h"

class RtkPortUI : public QWidget
{
	Q_OBJECT

public:
	RtkPortUI(QWidget *parent = nullptr);
	~RtkPortUI();
	void loadConfig(QJsonObject& json);
	void saveConfig(QJsonObject& json);
public slots:
	void onStartClicked();
	void onEnable(bool enable);
	void onDataSize(const int len);
private:
	Ui::RtkPortUIClass ui;
	uint64_t m_DataSize;
};
