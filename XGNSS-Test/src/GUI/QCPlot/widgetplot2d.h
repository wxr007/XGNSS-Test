#pragma once

#include <QWidget>
#include <QElapsedTimer>
#include "ui_widgetplot2d.h"
#include "qcustomplot.h"

class widgetplot2d : public QWidget
{
	Q_OBJECT

public:
	widgetplot2d(QWidget *parent = nullptr);
	~widgetplot2d();
	void initGraphName(QStringList name);
	void setYRange(double lower, double upper);
	void setXRange(double lower, double upper);
	void setXLabel(QString label);
	void setYLabel(QString label);
	void setXTimeTicker();
	void addData(QString name, const QVector<double>& keys, const QVector<double>& values);
	void clear();
protected:
	void initQCP();
	void setTheme(QColor axis, QColor background);
public slots:
	void addDataByTime(QString name, double data);
	void OnPlotClick(QCPAbstractPlottable* plottable, int dataIndex, QMouseEvent* event);
private:
	Ui::widgetplot2dClass ui;
	QMap<QString, int> nameToGraphMap; // 曲线名称对应曲线序号
	QVector<double> valueVector; // 存储曲线的当前值
	QElapsedTimer time;

	QCPItemTracer* tracer;
	QCPItemText* TextTip;
};
