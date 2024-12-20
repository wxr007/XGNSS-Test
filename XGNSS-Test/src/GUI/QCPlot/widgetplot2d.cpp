#include "widgetplot2d.h"

widgetplot2d::widgetplot2d(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    initQCP();

    tracer = new QCPItemTracer(ui.customPlot);  //生成游标
    //tracer->setPen(QPen(Qt::DashLine));         //游标线型：虚线
    //tracer->setStyle(QCPItemTracer::tsPlus);    //游标样式：十字星、圆圈、方框等
    tracer->setPen(QPen(Qt::red));
    tracer->setBrush(QBrush(Qt::red));
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setSize(5.0);

    // tracerLabel = new QCPItemText(ui.customPlot); //生成游标说明
    ////下面的代码就是设置游标说明的外观和对齐方式等状态
    // tracerLabel->setLayer("overlay");
    // tracerLabel->setPen(QPen(Qt::red));
    // tracerLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignTop);
    // //下面这个语句很重要，它将游标说明锚固在tracer位置处，实现自动跟随
    // tracerLabel->position->setParentAnchor(tracer->position);

    TextTip = new QCPItemText(ui.customPlot);//生成游标说明
    TextTip->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
    TextTip->position->setType(QCPItemPosition::ptAbsolute);
    QFont font;
    font.setPixelSize(15);
    TextTip->setFont(font); // make font a bit larger
    TextTip->setPen(QPen(Qt::black)); // show black border around text
    TextTip->setBrush(Qt::white);
    TextTip->setVisible(false);

    connect(ui.customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*, int, QMouseEvent*)), this, SLOT(OnPlotClick(QCPAbstractPlottable*, int, QMouseEvent*)));//关联选点信号
}
widgetplot2d::~widgetplot2d()
{}

void widgetplot2d::initQCP()
{
    // 显示图例
    ui.customPlot->legend->setVisible(true);
    ui.customPlot->legend->setBrush(QColor(255, 255, 255, 150));
    // 刻度显示
    ui.customPlot->xAxis->setTicks(true);
    ui.customPlot->yAxis->setTicks(true);
    // 刻度值显示
    ui.customPlot->xAxis->setTickLabels(true);
    ui.customPlot->yAxis->setTickLabels(true);
    // 网格显示
    ui.customPlot->xAxis->grid()->setVisible(true);
    ui.customPlot->yAxis->grid()->setVisible(true);
    // 子网格显示
    ui.customPlot->xAxis->grid()->setSubGridVisible(false);
    ui.customPlot->yAxis->grid()->setSubGridVisible(false);
    // 右和上坐标轴、刻度值显示
    ui.customPlot->xAxis2->setVisible(true);
    ui.customPlot->yAxis2->setVisible(true);
    ui.customPlot->yAxis2->setTicks(true);
    ui.customPlot->yAxis2->setTickLabels(true);

    connect(ui.customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui.customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // 暗色主题
    //setTheme(Qt::white, Qt::black);
    // 亮色主题
    setTheme(Qt::black, Qt::white);
    // 设置customplot的plottable绘图层可选  鼠标拖拽 滚轮缩放
    ui.customPlot->setInteractions( QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectLegend);
    //ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    //设置legend只能选择图例
    ui.customPlot->legend->setSelectableParts(QCPLegend::spItems);

    time.start();
}

void widgetplot2d::setTheme(QColor axis, QColor background)
{
    // 下方和左边
    // 坐标标注颜色
    ui.customPlot->xAxis->setLabelColor(axis);
    ui.customPlot->yAxis->setLabelColor(axis);
    // 坐标刻度值颜色
    ui.customPlot->xAxis->setTickLabelColor(axis);
    ui.customPlot->yAxis->setTickLabelColor(axis);
    // 坐标基线颜色和宽度
    ui.customPlot->xAxis->setBasePen(QPen(axis, 1));
    ui.customPlot->yAxis->setBasePen(QPen(axis, 1));
    // 坐标主刻度颜色和宽度
    ui.customPlot->xAxis->setTickPen(QPen(axis, 1));
    ui.customPlot->yAxis->setTickPen(QPen(axis, 1));
    // 坐标子刻度颜色和宽度
    ui.customPlot->xAxis->setSubTickPen(QPen(axis, 1));
    ui.customPlot->yAxis->setSubTickPen(QPen(axis, 1));

    // 上方和右边
    // 坐标标注颜色
    ui.customPlot->xAxis2->setLabelColor(axis);
    ui.customPlot->yAxis2->setLabelColor(axis);
    // 坐标刻度值颜色
    ui.customPlot->xAxis2->setTickLabelColor(axis);
    ui.customPlot->yAxis2->setTickLabelColor(axis);
    // 坐标基线颜色和宽度
    ui.customPlot->xAxis2->setBasePen(QPen(axis, 1));
    ui.customPlot->yAxis2->setBasePen(QPen(axis, 1));
    // 坐标主刻度颜色和宽度
    ui.customPlot->xAxis2->setTickPen(QPen(axis, 1));
    ui.customPlot->yAxis2->setTickPen(QPen(axis, 1));
    // 坐标子刻度颜色和宽度
    ui.customPlot->xAxis2->setSubTickPen(QPen(axis, 1));
    ui.customPlot->yAxis2->setSubTickPen(QPen(axis, 1));

    // 整个画布背景色
    ui.customPlot->setBackground(background);
    // 绘图区域背景色
    ui.customPlot->axisRect()->setBackground(background);
    // 刷新绘图
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
        // 添加曲线
        ui.customPlot->addGraph();
        ui.customPlot->graph(ui.customPlot->graphCount() - 1)->setPen(QPen(QColor(r, g, b),1.2));
        ui.customPlot->graph(ui.customPlot->graphCount() - 1)->setVisible(true);
        // 将曲线名称与曲线序号一一对应，之后添加数据就可以一一对应
        ui.customPlot->graph(i)->setName(name.at(i));//设置名称
        nameToGraphMap[name.at(i)] = i;
        // 初始化数据容器的值
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
    // x轴以时间形式显示
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

//选点处理函数
void widgetplot2d::OnPlotClick(QCPAbstractPlottable * plottable, int dataIndex, QMouseEvent * event)
{
  //先获取点击的绘图层名称,然后通过名称找到图层ID,再找到对应的数据点  这里因为知道ID 所以直接使用 没有通过名称找
  const QCPGraphData * ghd = ui.customPlot->graph(0)->data()->at(dataIndex);
  tracer->setGraph(ui.customPlot->graph(0));//设置游标吸附在traceGraph这条曲线上
  tracer->setGraphKey(ghd->key);//设置游标的X值（这就是游标随动的关键代码）

  QString text = QString("x=%1\ny=%2").arg(ghd->key).arg(ghd->value); // "(" + QString::number(ghd->key, 10, 0) + "," + QString::number(ghd->value, 10, 0) + ")";
  TextTip->setText(text);//文本内容填充
  TextTip->position->setCoords(event->pos().x(), event->pos().y() - 50);//文本框所在位置
  TextTip->setVisible(true);
  ui.customPlot->replot();
}