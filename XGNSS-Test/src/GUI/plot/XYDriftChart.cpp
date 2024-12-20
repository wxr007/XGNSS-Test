#include "XYDriftChart.h"
#include <QVBoxLayout>
#include "DataCache.h"
#include "ThreadManager.h"

XYDriftChart::XYDriftChart(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    setupUI();
    setupChart();

    //connect(ThreadManager::Instance().m_ReadFileThread, SIGNAL(sgnFinished()), this, SLOT(updateXYValues()), Qt::QueuedConnection);
}

XYDriftChart::~XYDriftChart()
{}

void XYDriftChart::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    chartView = new QChartView();
    layout->addWidget(chartView);
    setLayout(layout);
}

void XYDriftChart::setupChart()
{
    // 创建一个 QChart 实例
    QChart* chart = new QChart();
    //chart->setTheme(QChart::ChartThemeQt);
    //chart->setAnimationOptions(QChart::NoAnimation);
    //chart->setBackgroundVisible(false);
    //chart->setMargins(QMargins(0, 0, 0, 0));

    chart->setBackgroundBrush(Qt::gray);
    // 创建 QScatterSeries 实例
    int markerSize = 2;
    signalSeries = new QScatterSeries();
    // 设置散点的外观，如大小和颜色
    signalSeries->setName("Single");
    signalSeries->setPen(QPen(Qt::NoPen));   //不画轮廓
    signalSeries->setMarkerSize(markerSize); // 设置标记的大小
    signalSeries->setColor(Qt::red);  // 设置标记的颜色
    //signalSeries->setBorderColor(Qt::red);  // 设置轮廓的颜色
    dGPSSeries = new QScatterSeries();
    dGPSSeries->setName("dGPS");
    dGPSSeries->setPen(QPen(Qt::NoPen));   //不画轮廓
    dGPSSeries->setMarkerSize(markerSize); // 设置标记的大小
    dGPSSeries->setColor(Qt::blue);  // 设置标记的颜色
    //dGPSSeries->setBorderColor(Qt::blue);  // 设置轮廓的颜色
    floatSeries = new QScatterSeries();
    floatSeries->setName("float");
    floatSeries->setPen(QPen(Qt::NoPen));   //不画轮廓
    floatSeries->setMarkerSize(markerSize); // 设置标记的大小
    floatSeries->setColor(Qt::yellow);  // 设置标记的颜色
    //floatSeries->setBorderColor(Qt::yellow);  // 设置轮廓的颜色
    fixedSeries = new QScatterSeries();
    fixedSeries->setName("fixed");
    fixedSeries->setPen(QPen(Qt::NoPen));   //不画轮廓
    fixedSeries->setMarkerSize(markerSize); // 设置标记的大小
    fixedSeries->setColor(Qt::green);  // 设置标记的颜色
    //fixedSeries->setBorderColor(Qt::green);  // 设置轮廓的颜色
    insSeries = new QScatterSeries();
    insSeries->setName("INS");
    insSeries->setPen(QPen(Qt::NoPen));   //不画轮廓
    insSeries->setMarkerSize(markerSize); // 设置标记的大小
    insSeries->setColor(Qt::magenta);  // 设置标记的颜色
    //insSeries->setBorderColor(Qt::magenta);  // 设置轮廓的颜色

    // 添加数据点到散点系列
    //signalSeries->append(1, 6);
    //signalSeries->append(2, 4);
    //signalSeries->append(3, 8);
    //signalSeries->append(7, 4);
    //signalSeries->append(9, 5);

    // 将散点系列添加到图表中
    chart->addSeries(signalSeries);
    chart->addSeries(dGPSSeries);
    chart->addSeries(floatSeries);
    chart->addSeries(fixedSeries);
    chart->addSeries(insSeries);

    // 创建轴
    axisX = new QValueAxis();
    axisX->setLabelFormat("%.3f");
    axisX->setTitleText("X");
    axisX->setRange(0, 10);
    axisX->setMinorTickCount(2); // 次分隔个数
    chart->addAxis(axisX, Qt::AlignBottom);
    signalSeries->attachAxis(axisX);
    dGPSSeries->attachAxis(axisX);
    floatSeries->attachAxis(axisX);
    fixedSeries->attachAxis(axisX);
    insSeries->attachAxis(axisX);

    axisY = new QValueAxis();
    axisY->setLabelFormat("%.3f");
    axisY->setTitleText("Y");
    axisY->setRange(0, 10);
    axisY->setMinorTickCount(2); // 次分隔个数
    chart->addAxis(axisY, Qt::AlignLeft);
    signalSeries->attachAxis(axisY);
    dGPSSeries->attachAxis(axisY);
    floatSeries->attachAxis(axisY);
    fixedSeries->attachAxis(axisY);
    insSeries->attachAxis(axisY);

    // 使用 QChartView 显示图表
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setRubberBand(QChartView::RectangleRubberBand);
}

void XYDriftChart::updateXYValues()
{
    chartView->setUpdatesEnabled(false);
    qDebug("updateXYValues start");
    QList<QPointF>& signalList = DataCache::Instance().m_GGA2ZYZCache.signalList;
    QList<QPointF>& dGPSList = DataCache::Instance().m_GGA2ZYZCache.dGPSList;
    QList<QPointF>& floatList = DataCache::Instance().m_GGA2ZYZCache.floatList;
    QList<QPointF>& fixedList = DataCache::Instance().m_GGA2ZYZCache.fixedList;
    QList<QPointF>& insList = DataCache::Instance().m_GGA2ZYZCache.insList;
    if (signalList.size() == 0 && dGPSList.size() == 0 && floatList.size() == 0 && fixedList.size() == 0 && insList.size() == 0) {
        chartView->setUpdatesEnabled(true);
        return;
    }
    signalSeries->clear();
    dGPSSeries->clear();
    floatSeries->clear();
    fixedSeries->clear();
    insSeries->clear();

    double minX = DataCache::Instance().m_GGA2ZYZCache.minX, maxX = DataCache::Instance().m_GGA2ZYZCache.maxX;
    double minY = DataCache::Instance().m_GGA2ZYZCache.minY, maxY = DataCache::Instance().m_GGA2ZYZCache.maxY;

    signalSeries->replace(signalList);
    dGPSSeries->replace(dGPSList);
    floatSeries->replace(floatList);
    fixedSeries->replace(fixedList);
    insSeries->replace(insList);

    double deltaX = maxX - minX;
    double deltaY = maxY - minY;
    axisX->setRange(minX - deltaX * 0.1, maxX + deltaX * 0.1);
    axisY->setRange(minY - deltaY * 0.1, maxY + deltaY * 0.1);
    qDebug("updateXYValues end");
    chartView->setUpdatesEnabled(true);
}