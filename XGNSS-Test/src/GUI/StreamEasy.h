#pragma once

#include <QWidget>
#include "ui_StreamEasy.h"

QT_BEGIN_NAMESPACE
namespace Ui { class StreamEasyClass; };
QT_END_NAMESPACE

class StreamEasy : public QWidget
{
	Q_OBJECT

public:
	StreamEasy(QWidget *parent = nullptr);
	~StreamEasy();
	void setSettingAction(QAction *action);
public slots:
	void onStartClicked();
	void onEnable(bool enable);
private:
	Ui::StreamEasyClass *ui;
signals:
	void sgnStartSerialport();
	void sgnStartTcpClient();
	void sgnStartNtrip();
	void sgnSettingToggled(bool);
};
