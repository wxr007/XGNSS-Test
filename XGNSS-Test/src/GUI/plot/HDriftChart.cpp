#include "HDriftChart.h"
#include <QVBoxLayout>
#include "DataCache.h"
#include "ThreadManager.h"

HDriftChart::HDriftChart(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    setupUI();
    setupChart();

    //connect(ThreadManager::Instance().m_ReadFileThread,SIGNAL(sgnFinished()),this,SLOT(updateHValues()), Qt::QueuedConnection);
}

HDriftChart::~HDriftChart()
{}

void HDriftChart::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    chartView = new QChartView();
    layout->addWidget(chartView);
    setLayout(layout);
}

void HDriftChart::setupChart()
{
    // 创建一个 QChart 实例
    QChart* chart = new QChart();
    chart->setBackgroundBrush(Qt::gray);

    // 创建一个 QScatterSeries 实例
    int markerSize = 1;
    //m_HValueSeries = new QScatterSeries();
    m_HValueSeries = new QLineSeries();
    // 设置折现的外观，如大小和颜色
    m_HValueSeries->setName("H value");
    m_HValueSeries->setColor(Qt::red);  // 设置标记的颜色
    QPen pen = m_HValueSeries->pen();
    pen.setWidth(markerSize);
    m_HValueSeries->setPen(pen);
    //m_HValueSeries->setPen(QPen(Qt::NoPen));   //不画轮廓
    //m_HValueSeries->setMarkerSize(markerSize); // 设置标记的大小
    //m_HValueSeries->setBorderColor(Qt::red);  // 设置轮廓的颜色

    // 添加数据点到折现系列
    //m_HValueSeries->append(1, 6);
    //m_HValueSeries->append(2, 4);
    //m_HValueSeries->append(3, 8);
    //m_HValueSeries->append(7, 4); 
    //m_HValueSeries->append(9, 5);

    // 将折现系列添加到图表中
    chart->addSeries(m_HValueSeries);

    // 创建轴
    axisX = new QValueAxis();
    axisX->setLabelFormat("%i");
    axisX->setTitleText("X");
    axisX->setRange(0, 10);
    axisX->setMinorTickCount(2); // 次分隔个数
    chart->addAxis(axisX, Qt::AlignBottom);
    m_HValueSeries->attachAxis(axisX);

    axisY = new QValueAxis();
    axisY->setLabelFormat("%.3f");
    axisY->setTitleText("Y");
    axisY->setRange(0, 10);
    axisY->setMinorTickCount(2); // 次分隔个数
    chart->addAxis(axisY, Qt::AlignLeft);
    m_HValueSeries->attachAxis(axisY);

    // 使用 QChartView 显示图表
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setRubberBand(QChartView::RectangleRubberBand);
}

void HDriftChart::updateHValues()
{
    chartView->setUpdatesEnabled(false);
    QList<QPointF>& HVuleList = DataCache::Instance().m_GGA2ZYZCache.HValueList;
    if (HVuleList.size() == 0) {
        chartView->setUpdatesEnabled(true);
        return;
    }
    double minT = DataCache::Instance().m_GGA2ZYZCache.minT;
    double maxT = DataCache::Instance().m_GGA2ZYZCache.maxT;
    double minH = DataCache::Instance().m_GGA2ZYZCache.minH;
    double maxH = DataCache::Instance().m_GGA2ZYZCache.maxH;

    m_HValueSeries->replace(HVuleList);
    double deltaT = maxT - minT;
    axisX->setRange(minT - deltaT * 0.1, maxT + deltaT * 0.1);
    double deltaH = maxH - minH;
    axisY->setRange(minH - deltaH * 0.1, maxH + deltaH * 0.1);
    chartView->setUpdatesEnabled(true);
}
