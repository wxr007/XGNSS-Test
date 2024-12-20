#pragma once

#include <QWidget>
#include "ui_XYDriftChart.h"
#include <QChartView>
#include <QChart>
#include <QScatterSeries>
#include <QValueAxis>

QT_CHARTS_USE_NAMESPACE

class XYDriftChart : public QWidget
{
	Q_OBJECT

public:
	XYDriftChart(QWidget *parent = nullptr);
	~XYDriftChart();
	void setupUI();
	void setupChart();
public slots:
	void updateXYValues();
private:
	Ui::XYDriftChartClass ui;
	QChartView* chartView;
	QValueAxis* axisX;
	QValueAxis* axisY;
	QScatterSeries* signalSeries;
	QScatterSeries* dGPSSeries;
	QScatterSeries* floatSeries;
	QScatterSeries* fixedSeries;
	QScatterSeries* insSeries;
};
