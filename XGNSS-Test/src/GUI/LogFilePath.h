#pragma once

#include <QWidget>
#include "ui_LogFilePath.h"
#include <QJsonObject>

class LogFilePath : public QWidget
{
	Q_OBJECT

public:
	LogFilePath(QWidget *parent = nullptr);
	~LogFilePath();
	void saveConfig(QJsonObject& config);
	void loadConfig(QJsonObject& config);
public slots:
	void onOpenFileClicked();
	void onSelectPathClicked();
	void onSaveFileChecked(bool enable);
private:
	Ui::LogFilePathClass ui;
};
