#pragma once

#include <QWidget>
#include "ui_HVChart.h"
#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QDateTime>
#include <QTimer>

QT_CHARTS_USE_NAMESPACE

class HVChart : public QWidget
{
	Q_OBJECT

public:
	HVChart(QWidget *parent = nullptr);
	~HVChart();
	void initValues();
	void setupUI();
	void setupChart();
	void setupTimer();
	void updateAxis();
	void updateHV(double newHRMS, double newVRMS);
public slots:
	void onUpdateNMEA(int type);
private:
	Ui::HVChartClass ui;
	QChartView* chartView;
	QDateTimeAxis* axisX;
	QValueAxis* axisY_H;
	QValueAxis* axisY_V;
	QLineSeries* seriesHRMS;
	QLineSeries* seriesVRMS;
	double mMinHRMS;
	double mMaxHRMS;
	double mMinVRMS;
	double mMaxVRMS;
};
