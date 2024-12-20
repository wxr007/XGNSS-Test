#include "QCPHDriftChart.h"
#include "DataCache.h"

QCPHDriftChart::QCPHDriftChart(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initQCP();
	initGraph();
}

QCPHDriftChart::~QCPHDriftChart()
{}

void QCPHDriftChart::initQCP()
{
	// 显示图例
	ui.customPlot->legend->setVisible(true);
	ui.customPlot->legend->setBrush(QColor(180, 180, 180, 150)); // 透明度
	// x轴和y轴的标签
	ui.customPlot->xAxis->setLabel("T");
	ui.customPlot->yAxis->setLabel("H");
	// 右和上坐标轴、刻度值显示
	ui.customPlot->xAxis2->setVisible(true);//设置顶部轴线可见
	ui.customPlot->yAxis2->setVisible(true);//设置右侧轴线可见
	// 当修改左和底坐标轴时,使之与右和上坐标轴也跟着变化
	connect(ui.customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->xAxis2, SLOT(setRange(QCPRange)));
	connect(ui.customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->yAxis2, SLOT(setRange(QCPRange)));

	// 设置customplot的plottable绘图层可选  鼠标拖拽 滚轮缩放
	ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectLegend);

	// 绘图区域背景色
	ui.customPlot->axisRect()->setBackground(Qt::gray);
}

void QCPHDriftChart::initGraph()
{
	ui.customPlot->addGraph();
	ui.customPlot->graph(0)->setPen(QPen(Qt::red));
	ui.customPlot->graph(0)->setName("H Value");
}

void QCPHDriftChart::updateHValues()
{
	ui.customPlot->setUpdatesEnabled(false);
	QList<QPointF>& HVuleList = DataCache::Instance().m_GGA2ZYZCache.HValueList;
	QVector<double> x, y;
	for (int i = 0; i < HVuleList.size(); i++) {
		x.push_back(HVuleList[i].x());
		y.push_back(HVuleList[i].y());
	}
	double minT = DataCache::Instance().m_GGA2ZYZCache.minT;
	double maxT = DataCache::Instance().m_GGA2ZYZCache.maxT;
	double minH = DataCache::Instance().m_GGA2ZYZCache.minH;
	double maxH = DataCache::Instance().m_GGA2ZYZCache.maxH;

	ui.customPlot->xAxis->setRange(minT, maxT);
	ui.customPlot->yAxis->setRange(minH, maxH);
	ui.customPlot->graph(0)->setData(x,y);
	ui.customPlot->replot(QCustomPlot::rpImmediateRefresh);
	ui.customPlot->setUpdatesEnabled(true);
}
