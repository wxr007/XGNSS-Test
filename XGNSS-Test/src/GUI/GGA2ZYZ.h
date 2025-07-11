#pragma once

#include <QWidget>
#include <QTime>
#include "ui_GGA2ZYZ.h"
#include "plot/HDriftChart.h"
#include "plot/XYDriftChart.h"
#include "QCPlot/QCPHDriftChart.h"
#include "QCPlot/QCPXYDriftChart.h"

#define USE_QCP //QCustomPlot

class GGA2ZYZ : public QWidget
{
	Q_OBJECT

public:
	GGA2ZYZ(QWidget *parent = nullptr);
	~GGA2ZYZ();
	void saveSettings();
    void loadSettings();
public slots:
	void onSelectPathClicked();
	void onPlanarProjection();
	void onComparison();
	void onOpenCharts();
	void onProcess(int present, int msecs);
	void onFinished(QString filename);
	void onClear();
protected:
	void createHDrift(QString filename);
	void createXYDrift(QString filename);
	void closeAllChidren();
private:
	Ui::GGA2ZYZClass ui;
	QTime m_TimeShow;
#ifdef USE_QCP
	QMap<QString, QCPHDriftChart*> m_HDriftMap;
	QMap<QString, QCPXYDriftChart*> m_XYDriftMap;
#else
	QMap<QString, HDriftChart*> m_HDriftMap;
	QMap<QString, XYDriftChart*> m_XYDriftMap;
#endif // USE_QCP
};
