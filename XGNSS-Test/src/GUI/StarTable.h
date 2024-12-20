#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include "ui_StarTable.h"
#include "DataCache.h"

class StarTable : public QWidget
{
	Q_OBJECT

public:
	StarTable(QWidget *parent = nullptr);
	~StarTable();
protected:
	void setTableView(QTableView* table);
	void setModelItem(QStandardItemModel* model, int row, int colum, QString str);
	void setModelValue(QStandardItemModel* model, int row, int colum, double value, int prec = 1);
	void setModelValue(QStandardItemModel* model, int row, int colum, int value);

	void setTableUpdatesEnabled(bool enable);
	void setGPSData(std::vector<UnfoldMsg309Sat>& gps_stas);
	void setGLOData(std::vector<UnfoldMsg309Sat>& gol_stas);
	void setGALData(std::vector<UnfoldMsg309Sat>& gal_stas);
	void setBDSData(std::vector<UnfoldMsg309Sat>& bds_stas);
	void setQZSData(std::vector<UnfoldMsg309Sat>& qzs_stas);
	void setIRNData(std::vector<UnfoldMsg309Sat>& irn_stas);
public slots:
	void onUpdateHemisphereMsg(int type);
	void onAntennaChanged();
private:
	Ui::StarTableClass ui;
	UnfoldMsg309 unfoldMsg309;
	QStandardItemModel* mGPSM;
	QStandardItemModel* mGALM;
	QStandardItemModel* mGLOM;
	QStandardItemModel* mBDSM;
	QStandardItemModel* mQZSM;
	QStandardItemModel* mIRNM;
signals:
	void sgnSetAntenna(int);
};
