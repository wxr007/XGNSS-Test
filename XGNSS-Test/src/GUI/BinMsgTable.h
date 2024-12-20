#pragma once

#include <QWidget>
#include "ui_BinMsgTable.h"
#include "DataCache.h"

class BinMsgTable : public QWidget
{
	Q_OBJECT

public:
	BinMsgTable(QWidget *parent = nullptr);
	~BinMsgTable();
protected:
	void setTableBase(QTableWidget* table);
	void initBIN3Rows();
	void initBIN111Rows();
	void initBIN112Rows();
	void setItemString(QTableWidget* table, int row,int colum, QString str);
	void setItemInt(QTableWidget* table, int row,int colum, const int value);
	void setItemValueString(QTableWidget* table,int row, QString str);
	void setItemValue(QTableWidget* table, int row, double value, int prec);
	void setItemValue(QTableWidget* table, int row, int value);
	void setMsg3TableData(SBinaryMsg3* sMsg003);
	void setMsg111TableData(SBinaryMsg111* sMsg111);
	void setMsg112TableData(SBinaryMsg112* sMsg112);
	void updataMsgCountTable();
public slots:
	void onUpdateHemisphereMsg(int type);
private:
	Ui::BinMsgTableClass ui;
};
