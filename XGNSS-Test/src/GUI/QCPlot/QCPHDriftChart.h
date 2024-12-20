#pragma once

#include <QWidget>
#include "ui_QCPHDriftChart.h"

class QCPHDriftChart : public QWidget
{
	Q_OBJECT

public:
	QCPHDriftChart(QWidget *parent = nullptr);
	~QCPHDriftChart();
	void initQCP();
	void initGraph();
	void updateHValues();
private:
	Ui::QCPHDriftChartClass ui;
};
