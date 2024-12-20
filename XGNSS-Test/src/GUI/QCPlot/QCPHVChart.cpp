#include "QCPHVChart.h"
#include "DataCache.h"
#include "ThreadManager.h"
#include <QFontDataBase>

QCPHVChart::QCPHVChart(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    m_nKeepSeconds = 30;
    m_baseX = 0;
    m_baseY = 0;
    m_basePlane = 0;
    m_baseHeight = 0;
	initQCP();
    initGraph();
    //引入图形字体
    int fontId = QFontDatabase::addApplicationFont(":/XGNSSTest/res/fontawesome-webfont.ttf");
    QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont iconFont = QFont(fontName);
    ui.pause_btn->setFont(iconFont);
    ui.pause_btn->setText(QChar(0xf04c));

    connect(ui.pause_btn, SIGNAL(toggled(bool)), this, SLOT(onPause(bool)));
    connect(ui.set_btn, SIGNAL(clicked()), this, SLOT(onSetBtnClicked()));
    connect(ui.set_btn_2, SIGNAL(clicked()), this, SLOT(onSetXYHBtnClicked()));

    connect(ThreadManager::Instance().m_DataParser, SIGNAL(sgnUpdateNMEA(int)), this, SLOT(onUpdateNMEA(int)), Qt::QueuedConnection);

    ui.pause_btn->setToolTip("Pause or resume.");
    ui.pause_btn->setChecked(false);
    ui.time_range->setValue(m_nKeepSeconds);
    setStatus();
}

QCPHVChart::~QCPHVChart()
{}

void QCPHVChart::initQCP()
{
    // 显示图例
    ui.customPlot->legend->setVisible(true);
    ui.customPlot->legend->setBrush(QColor(255, 255, 255, 150)); // 透明度
    // 右和上坐标轴、刻度值显示
    //ui.customPlot->xAxis2->setVisible(true);//设置顶部轴线可见
    ui.customPlot->yAxis2->setVisible(true);//设置右侧轴线可见
    // x轴
    //ui.customPlot->axisRect()->setupFullAxesBox();
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui.customPlot->xAxis->setTicker(timeTicker);
    QTime now = QTime::currentTime();
    ui.customPlot->xAxis->setRange((double)now.msecsSinceStartOfDay() / 1000.0, m_nKeepSeconds, Qt::AlignRight);
    // 设置两个y轴
    ui.customPlot->yAxis->setTickLabels(false);
    connect(ui.customPlot->yAxis2, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->yAxis, SLOT(setRange(QCPRange))); // left axis only mirrors inner right axis
    ui.customPlot->yAxis2->setTickLabels(true);
    ui.customPlot->axisRect()->addAxis(QCPAxis::atRight);
    ui.customPlot->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(64); // add some padding to have space for tags
    ui.customPlot->axisRect()->axis(QCPAxis::atRight, 1)->setPadding(64); // add some padding to have space for tags

    // 设置customplot的plottable绘图层可选  鼠标 滚轮缩放
    ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectLegend);
}

void QCPHVChart::initGraph()
{
    ui.customPlot->addGraph(ui.customPlot->xAxis, ui.customPlot->axisRect()->axis(QCPAxis::atRight, 1));
    ui.customPlot->graph(0)->setPen(QPen(QColor(250, 120, 0)));// red line
    ui.customPlot->graph(0)->setName("Plane"); //水平

    ui.customPlot->addGraph(ui.customPlot->xAxis, ui.customPlot->axisRect()->axis(QCPAxis::atRight, 0));
    ui.customPlot->graph(1)->setPen(QPen(QColor(0, 180, 60)));// blue line
    ui.customPlot->graph(1)->setName("Vertical"); //高程

    mTag_H = new AxisTag(ui.customPlot->graph(0)->valueAxis());
    mTag_H->setPen(ui.customPlot->graph(0)->pen());
    mTag_V = new AxisTag(ui.customPlot->graph(1)->valueAxis());
    mTag_V->setPen(ui.customPlot->graph(1)->pen());
}

void QCPHVChart::updateHV(double newH, double newV)
{
    QTime now = QTime::currentTime();
    double key = (double)now.msecsSinceStartOfDay() / 1000.0;
    ui.customPlot->graph(0)->addData(key, newH);
    ui.customPlot->graph(1)->addData(key, newV);
    // remove data of lines that's outside visible range:
    ui.customPlot->graph(0)->data()->removeBefore(key - m_nKeepSeconds);
    ui.customPlot->graph(1)->data()->removeBefore(key - m_nKeepSeconds);
    // make key axis range scroll with the data:
    ui.customPlot->xAxis->rescale();
    bool isFound = false;
    QCPRange range1 = ui.customPlot->graph(0)->data()->valueRange(isFound);
    if (isFound) {
        double space = (range1.upper - range1.lower) * 0.05;
        ui.customPlot->axisRect()->axis(QCPAxis::atRight, 1)->setRange(range1.lower - space, range1.upper + space);
    }
    QCPRange range2 = ui.customPlot->graph(1)->data()->valueRange(isFound);
    if (isFound) {
        double space = (range2.upper - range2.lower) * 0.05;
        ui.customPlot->axisRect()->axis(QCPAxis::atRight, 0)->setRange(range2.lower - space, range2.upper + space);
    }
    //ui.customPlot->graph(0)->rescaleValueAxis(false, true);
    //ui.customPlot->graph(1)->rescaleValueAxis(false, true);
    ui.customPlot->xAxis->setRange(key, m_nKeepSeconds, Qt::AlignRight);
    // update the vertical axis tag positions and texts to match the rightmost data point of the graphs:
    mTag_H->updatePosition(newH);
    mTag_V->updatePosition(newV);
    mTag_H->setText(QString::number(newH, 'f', 5));
    mTag_V->setText(QString::number(newV, 'f', 5));

    ui.customPlot->replot();
    //ui.customPlot->replot(QCustomPlot::rpQueuedReplot);
}

void QCPHVChart::setStatus()
{
    ui.status->setText(QString::asprintf("Time range: %ds; Base point: x = %f, y = %f, h = %f", m_nKeepSeconds, m_baseX, m_baseY, m_baseHeight));
}

void QCPHVChart::onSetBtnClicked()
{
    m_nKeepSeconds = ui.time_range->value();
    setStatus();
}

void QCPHVChart::onSetXYHBtnClicked()
{
    QString x = ui.x->text();
    QString y = ui.y->text();
    QString h = ui.h->text();
    if (x.isEmpty() || y.isEmpty() || h.isEmpty()) {
        m_basePlane = 0;
        m_baseHeight = 0;
        QMessageBox::warning(this, "Warning", "Please input x,y,h");
        return;
    }
    m_baseX = x.toDouble();
    m_baseY = y.toDouble();
    m_baseHeight = h.toDouble();
    m_basePlane = sqrt(m_baseX * m_baseX + m_baseY * m_baseY);
    setStatus();
}

void QCPHVChart::onPause(bool checked)
{
    if (checked) {
        ui.pause_btn->setText(QChar(0xf04c));//pause
    }
    else {
        ui.pause_btn->setText(QChar(0xf04b));//go
    }
}

void QCPHVChart::onUpdateNMEA(int type)
{
    if (!ui.pause_btn->isChecked()) {
        return;
    }
    if ($GNGGA == type) {
        nmea_gga_t& m_nmea_gga = DataCache::Instance().m_nmea_gga;
        float height = m_nmea_gga.altitude + m_nmea_gga.undulation;
        BLHCoord blh(D2R(m_nmea_gga.lat), D2R(m_nmea_gga.lon), height);
        xyhCoord xyh = m_crd.BLH2xyh(blh);
        double Plane = sqrt(xyh.dE * xyh.dE + xyh.dN * xyh.dN);
        updateHV(Plane - m_basePlane, height - m_baseHeight);
    }
    //if ($GPGST == type) {
    //    nmea_gst_t& m_nmea_gst = DataCache::Instance().m_nmea_gst;
    //    double HRMS = sqrt(m_nmea_gst.lat_std * m_nmea_gst.lat_std + m_nmea_gst.lon_std * m_nmea_gst.lon_std);
    //    double VRMS = m_nmea_gst.height_std;
    //    updateHV(HRMS, VRMS);
    //}
}
