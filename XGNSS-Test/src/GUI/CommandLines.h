#pragma once

#include <QWidget>
#include "ui_CommandLines.h"

class CommandLines : public QWidget
{
	Q_OBJECT

public:
	CommandLines(QWidget *parent = nullptr);
	~CommandLines();
	void saveCommands();
	void loadCommands();
	void saveHistory();
	void loadHistory();
public slots:
	void onAddClicked();
	void onClearClicked();
	void onDoubleClicked();
	void onAddHistory(QString cmd);
	void onUboard_NMEAClicked();
	void onHSD_PORTUClicked();
	void onHSD_PORTIClicked();
private:
	Ui::CommandLinesClass ui;
signals:
	void sgnAddCmd(const QString);
	void sgnSendCmd(const QString);
};
