#pragma once

#include <QWidget>
#include "ui_TriggerCommand.h"

class TriggerCommand : public QWidget
{
	Q_OBJECT

public:
	TriggerCommand(QWidget *parent = nullptr);
	~TriggerCommand();

	void initValues();
	void totalPostType(int posType);
public slots:
	void onChangeTrigger(bool checked);
	void onUpdateNMEA(int type);
private:
	Ui::TriggerCommandClass ui;
	int mStatisticCount;
signals:
	void sgnAutoCommand(const QString);
};
