#include "widgetplot2d.h"

widgetplot2d::widgetplot2d(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    initQCP();

    tracer = new QCPItemTracer(ui.customPlot);  //�����α�
    //tracer->setPen(QPen(Qt::DashLine));         //�α����ͣ�����
    //tracer->setStyle(QCPItemTracer::tsPlus);    //�α���ʽ��ʮ���ǡ�ԲȦ�������
    tracer->setPen(QPen(Qt::red));
    tracer->setBrush(QBrush(Qt::red));
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setSize(5.0);

    // tracerLabel = new QCPItemText(ui.customPlot); //�����α�˵��
    ////����Ĵ�����������α�˵������ۺͶ��뷽ʽ��״̬
    // tracerLabel->setLayer("overlay");
    // tracerLabel->setPen(QPen(Qt::red));
    // tracerLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignTop);
    // //�������������Ҫ�������α�˵��ê����tracerλ�ô���ʵ���Զ�����
    // tracerLabel->position->setParentAnchor(tracer->position);

    TextTip = new QCPItemText(ui.customPlot);//�����α�˵��
    TextTip->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
    TextTip->position->setType(QCPItemPosition::ptAbsolute);
    QFont font;
    font.setPixelSize(15);
    TextTip->setFont(font); // make font a bit larger
    TextTip->setPen(QPen(Qt::black)); // show black border around text
    TextTip->setBrush(Qt::white);
    TextTip->setVisible(false);

    connect(ui.customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*, int, QMouseEvent*)), this, SLOT(OnPlotClick(QCPAbstractPlottable*, int, QMouseEvent*)));//����ѡ���ź�
}
widgetplot2d::~widgetplot2d()
{}

void widgetplot2d::initQCP()
{
    // ��ʾͼ��
    ui.customPlot->legend->setVisible(true);
    ui.customPlot->legend->setBrush(QColor(255, 255, 255, 150));
    // �̶���ʾ
    ui.customPlot->xAxis->setTicks(true);
    ui.customPlot->yAxis->setTicks(true);
    // �̶�ֵ��ʾ
    ui.customPlot->xAxis->setTickLabels(true);
    ui.customPlot->yAxis->setTickLabels(true);
    // ������ʾ
    ui.customPlot->xAxis->grid()->setVisible(true);
    ui.customPlot->yAxis->grid()->setVisible(true);
    // ��������ʾ
    ui.customPlot->xAxis->grid()->setSubGridVisible(false);
    ui.customPlot->yAxis->grid()->setSubGridVisible(false);
    // �Һ��������ᡢ�̶�ֵ��ʾ
    ui.customPlot->xAxis2->setVisible(true);
    ui.customPlot->yAxis2->setVisible(true);
    ui.customPlot->yAxis2->setTicks(true);
    ui.customPlot->yAxis2->setTickLabels(true);

    connect(ui.customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui.customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // ��ɫ����
    //setTheme(Qt::white, Qt::black);
    // ��ɫ����
    setTheme(Qt::black, Qt::white);
    // ����customplot��plottable��ͼ���ѡ  �����ק ��������
    ui.customPlot->setInteractions( QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectLegend);
    //ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    //����legendֻ��ѡ��ͼ��
    ui.customPlot->legend->setSelectableParts(QCPLegend::spItems);

    time.start();
}

void widgetplot2d::setTheme(QColor axis, QColor background)
{
    // �·������
    // �����ע��ɫ
    ui.customPlot->xAxis->setLabelColor(axis);
    ui.customPlot->yAxis->setLabelColor(axis);
    // ����̶�ֵ��ɫ
    ui.customPlot->xAxis->setTickLabelColor(axis);
    ui.customPlot->yAxis->setTickLabelColor(axis);
    // ���������ɫ�Ϳ��
    ui.customPlot->xAxis->setBasePen(QPen(axis, 1));
    ui.customPlot->yAxis->setBasePen(QPen(axis, 1));
    // �������̶���ɫ�Ϳ��
    ui.customPlot->xAxis->setTickPen(QPen(axis, 1));
    ui.customPlot->yAxis->setTickPen(QPen(axis, 1));
    // �����ӿ̶���ɫ�Ϳ��
    ui.customPlot->xAxis->setSubTickPen(QPen(axis, 1));
    ui.customPlot->yAxis->setSubTickPen(QPen(axis, 1));

    // �Ϸ����ұ�
    // �����ע��ɫ
    ui.customPlot->xAxis2->setLabelColor(axis);
    ui.customPlot->yAxis2->setLabelColor(axis);
    // ����̶�ֵ��ɫ
    ui.customPlot->xAxis2->setTickLabelColor(axis);
    ui.customPlot->yAxis2->setTickLabelColor(axis);
    // ���������ɫ�Ϳ��
    ui.customPlot->xAxis2->setBasePen(QPen(axis, 1));
    ui.customPlot->yAxis2->setBasePen(QPen(axis, 1));
    // �������̶���ɫ�Ϳ��
    ui.customPlot->xAxis2->setTickPen(QPen(axis, 1));
    ui.customPlot->yAxis2->setTickPen(QPen(axis, 1));
    // �����ӿ̶���ɫ�Ϳ��
    ui.customPlot->xAxis2->setSubTickPen(QPen(axis, 1));
    ui.customPlot->yAxis2->setSubTickPen(QPen(axis, 1));

    // ������������ɫ
    ui.customPlot->setBackground(background);
    // ��ͼ���򱳾�ɫ
    ui.customPlot->axisRect()->setBackground(background);
    // ˢ�»�ͼ
    ui.customPlot->replot();
}

void widgetplot2d::initGraphName(QStringList name)
{
    int r = 0, g = 0, b = 0;
    for (int i = 0; i < name.length(); i++) {
        r += 150; g += 50; b += 100;
        if (r > 255) r -= 255;
        if (g > 255) g -= 255;
        if (b > 255) b -= 255;
        // �������
        ui.customPlot->addGraph();
        ui.customPlot->graph(ui.customPlot->graphCount() - 1)->setPen(QPen(QColor(r, g, b),1.2));
        ui.customPlot->graph(ui.customPlot->graphCount() - 1)->setVisible(true);
        // �������������������һһ��Ӧ��֮��������ݾͿ���һһ��Ӧ
        ui.customPlot->graph(i)->setName(name.at(i));//��������
        nameToGraphMap[name.at(i)] = i;
        // ��ʼ������������ֵ
        for (int t = 0; t < name.length(); t++) {
            valueVector.append(0);
        }
    }
}

void widgetplot2d::setYRange(double lower, double upper)
{
    ui.customPlot->yAxis->setRange(lower, upper);
}

void widgetplot2d::setXRange(double lower, double upper)
{
    ui.customPlot->xAxis->setRange(lower, upper);
}

void widgetplot2d::setXLabel(QString label)
{
    ui.customPlot->xAxis->setLabel(label);
}

void widgetplot2d::setYLabel(QString label)
{
    ui.customPlot->yAxis->setLabel(label);
}

void widgetplot2d::setXTimeTicker()
{
    // x����ʱ����ʽ��ʾ
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui.customPlot->xAxis->setTicker(timeTicker);
    ui.customPlot->axisRect()->setupFullAxesBox();
    ui.customPlot->replot();
}

void widgetplot2d::addDataByTime(QString name, double value)
{
    //static double start = time.hour() * 60 * 60 + time.minute() * 60 + time.second() + time.msec() / 1000.0;
    double key = time.elapsed() / 1000.0;
    int timeAxis = 20;
    ui.customPlot->xAxis->setRange(key, timeAxis, Qt::AlignRight);
    ui.customPlot->graph(nameToGraphMap[name])->addData(key, value);
    ui.customPlot->replot(QCustomPlot::rpQueuedReplot);
}

void widgetplot2d::addData(QString name, const QVector<double>& keys, const QVector<double>& values) {
    ui.customPlot->graph(nameToGraphMap[name])->data().data()->clear();
    ui.customPlot->graph(nameToGraphMap[name])->addData(keys, values);
    ui.customPlot->replot(QCustomPlot::rpImmediateRefresh);
}

void widgetplot2d::clear()
{
    for (int var = 0; var < ui.customPlot->graphCount(); var++) {
        ui.customPlot->graph(var)->data().data()->clear();
    }
    ui.customPlot->replot(QCustomPlot::rpImmediateRefresh);
}

//ѡ�㴦����
void widgetplot2d::OnPlotClick(QCPAbstractPlottable * plottable, int dataIndex, QMouseEvent * event)
{
  //�Ȼ�ȡ����Ļ�ͼ������,Ȼ��ͨ�������ҵ�ͼ��ID,���ҵ���Ӧ�����ݵ�  ������Ϊ֪��ID ����ֱ��ʹ�� û��ͨ��������
  const QCPGraphData * ghd = ui.customPlot->graph(0)->data()->at(dataIndex);
  tracer->setGraph(ui.customPlot->graph(0));//�����α�������traceGraph����������
  tracer->setGraphKey(ghd->key);//�����α��Xֵ��������α��涯�Ĺؼ����룩

  QString text = QString("x=%1\ny=%2").arg(ghd->key).arg(ghd->value); // "(" + QString::number(ghd->key, 10, 0) + "," + QString::number(ghd->value, 10, 0) + ")";
  TextTip->setText(text);//�ı��������
  TextTip->position->setCoords(event->pos().x(), event->pos().y() - 50);//�ı�������λ��
  TextTip->setVisible(true);
  ui.customPlot->replot();
}