#pragma once

#include <QWidget>
#include "ui_QCPXYDriftChart.h"

class QCPXYDriftChart : public QWidget
{
	Q_OBJECT

public:
	QCPXYDriftChart(QWidget *parent = nullptr);
	~QCPXYDriftChart();
	void initQCP();
	void initGraph();
	void updateXYValues();
	void clear();
private:
	Ui::QCPXYDriftChartClass ui;
};
