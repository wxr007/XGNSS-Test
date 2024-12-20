#pragma once

#include <QWidget>
#include "ui_HDriftChart.h"
#include <QChartView>
#include <QChart>
#include <QScatterSeries>
#include <QLineSeries>
#include <QValueAxis>

QT_CHARTS_USE_NAMESPACE

class HDriftChart : public QWidget
{
	Q_OBJECT

public:
	HDriftChart(QWidget *parent = nullptr);
	~HDriftChart();
	void setupUI();
	void setupChart();
public slots:
	void updateHValues();
private:
	Ui::HDriftChartClass ui;
	QChartView* chartView;
	QValueAxis* axisX;
	QValueAxis* axisY;
	//QScatterSeries* m_HValueSeries;
	QLineSeries* m_HValueSeries;
};
