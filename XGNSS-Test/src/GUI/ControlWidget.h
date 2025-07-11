#pragma once

#include <QWidget>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include "ui_ControlWidget.h"
#include "CommandLines.h"

class ControlWidget : public QWidget
{
	Q_OBJECT

public:
	ControlWidget(QWidget *parent = nullptr);
	~ControlWidget();
	void saveConfig(QJsonObject& config);
	void loadConfig(QJsonObject& config);
public slots:
	void onCmdsClicked();
	void onSendClicked();
	void onSendCmd(const QString cmd);
	void onAddCmd(const QString cmd);
	void onClearClicked();
	void onShowDataLog(QByteArray log);
	void onShowMsg(int level, const QString log);
	void onChangeAutoSend();
	void onPause(bool);
	void onAutoSend();
private:
	Ui::ControlWidgetClass ui;
	CommandLines* m_Cmds;
	int m_MsgLines;
	QTimer m_AutoSendTimer;
signals:
	void sgnAddHistory(QString);
	void sgnWrite(const QByteArray data);
};
