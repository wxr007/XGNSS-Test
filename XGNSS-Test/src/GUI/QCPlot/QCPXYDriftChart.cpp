#include "QCPXYDriftChart.h"
#include "DataCache.h"

QCPXYDriftChart::QCPXYDriftChart(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    initQCP();
	initGraph();
}

QCPXYDriftChart::~QCPXYDriftChart()
{}

void QCPXYDriftChart::initQCP()
{
    // 显示图例
    ui.customPlot->legend->setVisible(true);
    ui.customPlot->legend->setBrush(QColor(180, 180, 180, 150)); // 透明度
    // x轴和y轴的标签
    ui.customPlot->xAxis->setLabel("X");
    ui.customPlot->yAxis->setLabel("Y");
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

void QCPXYDriftChart::initGraph()
{
    int markerSize = 2;
	ui.customPlot->addGraph();
	ui.customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, markerSize));
    ui.customPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
	ui.customPlot->graph(0)->setPen(QPen(Qt::red));
	ui.customPlot->graph(0)->setName("Single");

    ui.customPlot->addGraph();
    ui.customPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, markerSize));
    ui.customPlot->graph(1)->setLineStyle(QCPGraph::lsNone);
    ui.customPlot->graph(1)->setPen(QPen(Qt::blue));
    ui.customPlot->graph(1)->setName("dGPS");

    ui.customPlot->addGraph();
    ui.customPlot->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, markerSize));
    ui.customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui.customPlot->graph(2)->setPen(QPen(Qt::yellow));
    ui.customPlot->graph(2)->setName("float");

    ui.customPlot->addGraph();
    ui.customPlot->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, markerSize));
    ui.customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
    ui.customPlot->graph(3)->setPen(QPen(Qt::green));
    ui.customPlot->graph(3)->setName("fixed");

    ui.customPlot->addGraph();
    ui.customPlot->graph(4)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, markerSize));
    ui.customPlot->graph(4)->setLineStyle(QCPGraph::lsNone);
    ui.customPlot->graph(4)->setPen(QPen(Qt::magenta));
    ui.customPlot->graph(4)->setName("INS");
}

void QCPXYDriftChart::updateXYValues()
{
    ui.customPlot->setUpdatesEnabled(false);
    QList<QPointF>& signalList = DataCache::Instance().m_GGA2ZYZCache.signalList;
    QList<QPointF>& dGPSList = DataCache::Instance().m_GGA2ZYZCache.dGPSList;
    QList<QPointF>& floatList = DataCache::Instance().m_GGA2ZYZCache.floatList;
    QList<QPointF>& fixedList = DataCache::Instance().m_GGA2ZYZCache.fixedList;
    QList<QPointF>& insList = DataCache::Instance().m_GGA2ZYZCache.insList;
    if (signalList.size() == 0 && dGPSList.size() == 0 && floatList.size() == 0 && fixedList.size() == 0 && insList.size() == 0) {
        ui.customPlot->setUpdatesEnabled(true);
        return;
    }
    clear();
    double minX = DataCache::Instance().m_GGA2ZYZCache.minX, maxX = DataCache::Instance().m_GGA2ZYZCache.maxX;
    double minY = DataCache::Instance().m_GGA2ZYZCache.minY, maxY = DataCache::Instance().m_GGA2ZYZCache.maxY;
    QVector<double> signal_x, signal_y;
    for (int i = 0; i < signalList.size(); i++) {
        signal_x.append(signalList[i].x());
        signal_y.append(signalList[i].y());
    }
    ui.customPlot->graph(0)->setData(signal_x, signal_y);
    QVector<double> dGPS_x, dGPS_y;
    for (int i = 0; i < dGPSList.size(); i++) {
        dGPS_x.append(dGPSList[i].x());
        dGPS_y.append(dGPSList[i].y());
    }
    ui.customPlot->graph(1)->setData(dGPS_x, dGPS_y);
    QVector<double> float_x, float_y;
    for (int i = 0; i < floatList.size(); i++) {
        float_x.append(floatList[i].x());
        float_y.append(floatList[i].y());
    }
    ui.customPlot->graph(2)->setData(float_x, float_y);
    QVector<double> fixed_x, fixed_y;
    for (int i = 0; i < fixedList.size(); i++) {
        fixed_x.append(fixedList[i].x());
        fixed_y.append(fixedList[i].y());
    }
    ui.customPlot->graph(3)->setData(fixed_x, fixed_y);
    QVector<double> ins_x, ins_y;
    for (int i = 0; i < insList.size(); i++) {
        ins_x.append(insList[i].x());
        ins_y.append(insList[i].y());
    }
    ui.customPlot->graph(4)->setData(ins_x, ins_y);

    ui.customPlot->xAxis->setRange(minX, maxX);
    ui.customPlot->yAxis->setRange(minY, maxY);
    ui.customPlot->replot(QCustomPlot::rpImmediateRefresh);
    ui.customPlot->setUpdatesEnabled(true);
}

void QCPXYDriftChart::clear()
{
    for (int var = 0; var < ui.customPlot->graphCount(); var++) {
        ui.customPlot->graph(var)->data().data()->clear();
    }
}
