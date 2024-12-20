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
	// ��ʾͼ��
	ui.customPlot->legend->setVisible(true);
	ui.customPlot->legend->setBrush(QColor(180, 180, 180, 150)); // ͸����
	// x���y��ı�ǩ
	ui.customPlot->xAxis->setLabel("T");
	ui.customPlot->yAxis->setLabel("H");
	// �Һ��������ᡢ�̶�ֵ��ʾ
	ui.customPlot->xAxis2->setVisible(true);//���ö������߿ɼ�
	ui.customPlot->yAxis2->setVisible(true);//�����Ҳ����߿ɼ�
	// ���޸���͵�������ʱ,ʹ֮���Һ���������Ҳ���ű仯
	connect(ui.customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->xAxis2, SLOT(setRange(QCPRange)));
	connect(ui.customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->yAxis2, SLOT(setRange(QCPRange)));

	// ����customplot��plottable��ͼ���ѡ  �����ק ��������
	ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectLegend);

	// ��ͼ���򱳾�ɫ
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
