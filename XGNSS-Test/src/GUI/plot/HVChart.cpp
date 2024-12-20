#include "HVChart.h"
#include "DataCache.h"
#include <QVBoxLayout>
#include "ThreadManager.h"

HVChart::HVChart(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    initValues();
    setupUI();
    setupChart();
    //setupTimer();

    //connect(ThreadManager::Instance().m_DataParser, SIGNAL(sgnUpdateNMEA(int)), this, SLOT(onUpdateNMEA(int)), Qt::QueuedConnection);
}

HVChart::~HVChart()
{}

void HVChart::initValues()
{
    mMinHRMS = 0;
    mMaxHRMS = 1;
    mMinVRMS = 0;
    mMaxVRMS = 1;
}

void HVChart::onUpdateNMEA(int type) {
	if ($GPGST == type) {
		nmea_gst_t& m_nmea_gst = DataCache::Instance().m_nmea_gst;
		double HRMS = sqrt(m_nmea_gst.lat_std * m_nmea_gst.lat_std + m_nmea_gst.lon_std * m_nmea_gst.lon_std);
		double VRMS = m_nmea_gst.height_std;
        updateHV(HRMS, VRMS);
	}
}

void HVChart::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    chartView = new QChartView();
    layout->addWidget(chartView);
    setLayout(layout);
}

void HVChart::setupChart() {
    // 创建图表
    QChart* chart = new QChart();
    // 创建红色折线图系列
    seriesHRMS = new QLineSeries();
    seriesHRMS->setName("HRMS");
    seriesHRMS->setColor(Qt::red);
    seriesHRMS->setBrush(QBrush(Qt::red));
    seriesHRMS->setPointsVisible(true);

    // 创建绿色折线图系列
    seriesVRMS = new QLineSeries();
    seriesVRMS->setName("VRMS");
    seriesVRMS->setColor(Qt::blue);
    seriesVRMS->setBrush(QBrush(Qt::blue));

    // 初始化时间
    QDateTime now = QDateTime::currentDateTime();

    // 添加数据点到红色和绿色折线图系列
    //for (int i = 0; i < 60; i++) {
    //    seriesHRMS->append(now.addSecs(i-60).toMSecsSinceEpoch(), qrand() / double(RAND_MAX));
    //    seriesVRMS->append(now.addSecs(i-60).toMSecsSinceEpoch(), qrand() / double(RAND_MAX));
    //}

    //chart->legend()->hide();
    chart->addSeries(seriesHRMS);
    chart->addSeries(seriesVRMS);

    // 创建时间轴
    axisX = new QDateTimeAxis;
    axisX->setFormat("hh:mm:ss");
    axisX->setTitleText(u8"Time");
    chart->addAxis(axisX, Qt::AlignBottom);
    seriesHRMS->attachAxis(axisX);
    seriesVRMS->attachAxis(axisX);

    // 创建水平Y轴
    axisY_H = new QValueAxis;
    axisY_H->setLabelFormat("%0.2f");
    axisY_H->setTitleText(u8"Plane");
    //axisY_H->setMin(0);
    //axisY_H->setMax(2);
    axisY_H->setTickCount(10);
    chart->addAxis(axisY_H, Qt::AlignLeft);
    seriesHRMS->attachAxis(axisY_H);
    // 创建垂直Y轴
    axisY_V = new QValueAxis;
    axisY_V->setLabelFormat("%0.2f");
    axisY_V->setTitleText(u8"Vertical");
    //axisY_V->setMin(0);
    //axisY_V->setMax(1);
    axisY_V->setTickCount(10);
    chart->addAxis(axisY_V, Qt::AlignRight);
    seriesVRMS->attachAxis(axisY_V);

    // 设置轴范围
    axisX->setMin(now.addSecs(-60));
    axisX->setMax(now);

    // 设置图表视图
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setRubberBand(QChartView::RectangleRubberBand);
}

void HVChart::setupTimer() {
    // 设置定时器，使X轴随时间滚动
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &HVChart::updateAxis);
    timer->start(1000); // 每1000毫秒更新一次
}
//测试按时间更新
void HVChart::updateAxis() {
    QDateTime now = QDateTime::currentDateTime();

    axisX->setMin(now.addSecs(-60));// 保持1分钟的时间窗口
    axisX->setMax(now);

    double newHRMS = qrand() / double(RAND_MAX) * 2;
    seriesHRMS->append(now.toMSecsSinceEpoch(), newHRMS);
    if (seriesHRMS->count() > 60) {
        seriesHRMS->remove(0);
    }
    if (newHRMS < mMinHRMS) {
        mMinHRMS = newHRMS;
        axisY_H->setMin(mMinHRMS);
    }
    if (newHRMS > mMaxHRMS) {
        mMaxHRMS = newHRMS;
        axisY_H->setMax(mMaxHRMS);
    }
    double newVRMS = qrand() / double(RAND_MAX);
    seriesVRMS->append(now.toMSecsSinceEpoch(), newVRMS);
    if (seriesVRMS->count() > 60) {
        seriesVRMS->remove(0);
    }
    if (newVRMS < mMinVRMS) {
        mMinVRMS = newVRMS;
        axisY_V->setMin(mMinVRMS);
    }
    if (newVRMS > mMaxVRMS) {
        mMaxVRMS = newVRMS;
        axisY_V->setMax(mMaxVRMS);
    }
}

void HVChart::updateHV(double newHRMS, double newVRMS) {
    QDateTime now = QDateTime::currentDateTime();

    axisX->setMin(now.addSecs(-60));// 保持1分钟的时间窗口
    axisX->setMax(now);

    seriesHRMS->append(now.toMSecsSinceEpoch(), newHRMS);
    if (seriesHRMS->count() > 600) {
        seriesHRMS->remove(0);
    }
    if (newHRMS < mMinHRMS) {
        mMinHRMS = newHRMS;
        axisY_H->setMin(mMinHRMS);
    }
    if (newHRMS > mMaxHRMS) {
        mMaxHRMS = newHRMS;
        axisY_H->setMax(mMaxHRMS);
    }
    seriesVRMS->append(now.toMSecsSinceEpoch(), newVRMS);
    if (seriesVRMS->count() > 600) {
        seriesVRMS->remove(0);
    }
    if (newVRMS < mMinVRMS) {
        mMinVRMS = newVRMS;
        axisY_V->setMin(mMinVRMS);
    }
    if (newVRMS > mMaxVRMS) {
        mMaxVRMS = newVRMS;
        axisY_V->setMax(mMaxVRMS);
    }
}