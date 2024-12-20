#pragma once

#include <QWidget>
#include "ui_DataTableUI.h"
#include "Coordinate.h"
#include "StreamEasy.h"

class DataTableUI : public QWidget
{
	Q_OBJECT

public:
	DataTableUI(QWidget *parent = nullptr);
	~DataTableUI();
protected:
	void setTableBase(QTableWidget* table);
	void initTable1Rows();
	void initTable2Rows();
	void initValues();
	void setItemColor(QTableWidget* table,int row, int column, QColor color);
	void setItemString(QTableWidget* table, int row, QString str);
	void setItemValue(QTableWidget* table, int row, double value, int prec=1);
	void setItemValue(QTableWidget* table, int row, int value);
	void setItemValue(QTableWidget* table, int row, uint64_t value);
	void totalPostType(int posType);
	QString getPostTypeName(int posType);
public slots:
	void onOpen(bool enable);
	void onUpdateNMEA(int type);
	void onDataSize(const int len);
	void onClearCount();
	void onClearData();
private:
	Ui::DataTableUIClass ui;
	QVector<QColor> mPosTypeColor;
	CCoordinate m_crd;
	int mFixedNum;
	int mFloatNum;
	int mRtdNum;
	int mSingleNum;
	uint64_t mDataSize;
	int mMsgNum;
	double mDiffSum;
public:
	StreamEasy* mStreamEasy;
signals:
	void sgnWrite(const QByteArray data);
};
