#pragma once

#include <QDialog>
#include "ui_AboutDialog.h"

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	AboutDialog(QWidget *parent = nullptr, QString name = "", QString version = "");
	~AboutDialog();

private:
	Ui::AboutDialogClass ui;
};
