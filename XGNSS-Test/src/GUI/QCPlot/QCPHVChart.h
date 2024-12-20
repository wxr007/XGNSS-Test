#pragma once

#include <QWidget>
#include "ui_QCPHVChart.h"
#include "Coordinate.h"
#include "axistag.h"

class QCPHVChart : public QWidget
{
	Q_OBJECT

public:
	QCPHVChart(QWidget *parent = nullptr);
	~QCPHVChart();
protected:
	void initQCP();
	void initGraph();
	void updateHV(double newHRMS, double newVRMS);
	void setStatus();
public slots:
	void onUpdateNMEA(int type);
	void onSetBtnClicked();
	void onSetXYHBtnClicked();
	void onPause(bool checked);
private:
	Ui::QCPHVChartClass ui;
	int m_nKeepSeconds;
	CCoordinate m_crd;
	AxisTag* mTag_H;
	AxisTag* mTag_V;
	double m_baseX;
	double m_baseY;
	double m_basePlane;
	double m_baseHeight;
};
