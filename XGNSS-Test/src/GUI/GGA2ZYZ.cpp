#include "GGA2ZYZ.h"
#include <QScreen>
#include <QFileDialog>
#include <QMessageBox>
#include "ThreadManager.h"
#include "DataCache.h"

GGA2ZYZ::GGA2ZYZ(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	m_HDriftMap.clear();
    m_XYDriftMap.clear();

	connect(ui.m_ButtonSelect, SIGNAL(clicked()), this, SLOT(onSelectPathClicked()));
	connect(ui.planarProjection, SIGNAL(clicked()), this, SLOT(onPlanarProjection()));
	connect(ui.comparison, SIGNAL(clicked()), this, SLOT(onComparison()));
	connect(ui.clear_btn, SIGNAL(clicked()), this, SLOT(onClear()));
	connect(ui.listWidget, &QListWidget::itemDoubleClicked, this, &GGA2ZYZ::onOpenCharts);

	connect(ThreadManager::Instance().m_ReadFileThread, SIGNAL(sgnFinished(QString)), this, SLOT(onFinished(QString)),Qt::QueuedConnection);
	connect(ThreadManager::Instance().m_ReadFileThread, SIGNAL(sgnProgress(int, int)), this, SLOT(onProcess(int, int)), Qt::QueuedConnection);

	ui.listWidget->setAlternatingRowColors(true);	//开启交替行背景色,在设置style为交替颜色时必须开启
	ui.listWidget->setToolTip(u8"双击文件名，可打开关闭的图表。");
}

GGA2ZYZ::~GGA2ZYZ()
{
	ui.listWidget->clear();
	closeAllChidren();
}

void GGA2ZYZ::onSelectPathClicked()
{
	QString current_file = ui.m_EditFilePath->text();
	QStringList pathList = QFileDialog::getOpenFileNames(this, "Choose File", current_file);
	if (pathList.length() == 0) {
		return;
	}
	ui.m_EditFilePath->setText(pathList[0].left(pathList[0].lastIndexOf('/')));
	ui.listWidget->clear();
	for (int i = 0; i < pathList.length(); i++) {
		QString fileName = pathList[i].split("/").last();
		ui.listWidget->addItem(fileName);
		ui.listWidget->item(i)->setToolTip(pathList[i]);
	}
	closeAllChidren();
}

void GGA2ZYZ::onPlanarProjection()
{
	if (ThreadManager::Instance().m_ReadFileThread->isRunning()) {
		ThreadManager::Instance().m_ReadFileThread->stop();
	}
	else {
		//QString current_file = ui.m_EditFilePath->text();
		//if (current_file.isEmpty()) {
		//	QMessageBox::warning(this, u8"警告", u8"请先选择一个文件!");
		//	return;
		//}
		if (!ui.listWidget->count()) {
			QMessageBox::warning(this, u8"警告", u8"请先选择文件!");
		}
		QStringList pathList;
        for (int i = 0; i < ui.listWidget->count(); i++) {
			pathList.append(ui.listWidget->item(i)->toolTip());
        }
		ThreadManager::Instance().m_ReadFileThread->setFileNameList(pathList);
		ThreadManager::Instance().m_ReadFileThread->setFileFormat(ui.statement->currentIndex());
		ThreadManager::Instance().m_ReadFileThread->setSolType(ui.solutionStatus->currentIndex());
		ThreadManager::Instance().m_ReadFileThread->setKml(ui.kml->isChecked());
		if (ui.lat->text().isEmpty() || ui.lon->text().isEmpty() || ui.h->text().isEmpty()) {
			ThreadManager::Instance().m_ReadFileThread->setUseAvg();
		}
		else {
			ThreadManager::Instance().m_ReadFileThread->setBaseXYH(ui.lat->text().toDouble(), ui.lon->text().toDouble(), ui.h->text().toDouble());
		}
		ThreadManager::Instance().m_ReadFileThread->setDelayLimit(ui.delayLimit->text().toDouble());
		ThreadManager::Instance().m_ReadFileThread->setSecondOfMinute(ui.timeSlice->text().toInt());
		ThreadManager::Instance().m_ReadFileThread->setInterval(ui.interval->currentIndex());
		if (ui.startTime->text().isEmpty() || ui.cycleTime->text().isEmpty() || ui.breakPeriod->text().isEmpty()) {
			ThreadManager::Instance().m_ReadFileThread->setLoopTime("08000", 0, 0);
		}
		else {
			ThreadManager::Instance().m_ReadFileThread->setLoopTime(ui.startTime->text(), ui.cycleTime->text().toInt(), ui.breakPeriod->text().toInt());
		}
		ThreadManager::Instance().m_ReadFileThread->setCenter(ui.meridian->text().toDouble());
		ThreadManager::Instance().m_ReadFileThread->start();
	}
}

void GGA2ZYZ::onComparison()
{

}

void GGA2ZYZ::onOpenCharts()
{
	QListWidgetItem* currentItem = ui.listWidget->currentItem();
	if (currentItem != nullptr) {
		// 获取当前项的文本内容
		const QString itemText = currentItem->text();
#ifdef USE_QCP
		QCPHDriftChart* HDrift = m_HDriftMap.value(itemText);
#else
		HDriftChart* HDrift = m_HDriftMap.value(itemText);
#endif // USE_QCP
		if (HDrift != nullptr) {
			HDrift->show();
		}
#ifdef USE_QCP
        QCPXYDriftChart* XYDrift = m_XYDriftMap.value(itemText);
#else
		XYDriftChart* XYDrift = m_XYDriftMap.value(itemText);
#endif // USE_QCP
		if (XYDrift != nullptr) {
			XYDrift->show();
		}
	}
}

void GGA2ZYZ::onProcess(int present, int msecs)
{
	ui.progressBar->setValue(present);
	double dProgress = ui.progressBar->value() * 100.0 / ui.progressBar->maximum();
	ui.progressBar->setFormat(QString("%1%").arg(QString::number(dProgress, 'f', 2)));
	m_TimeShow.setHMS(0, 0, 0, 0);
	ui.time_lb->setText(m_TimeShow.addMSecs(msecs).toString("hh:mm:ss:zzz"));
}

void GGA2ZYZ::onFinished(QString filename)
{
	ThreadManager::Instance().m_ReadFileMutex.lock();
	QString line;
	QString message;
	ui.logText->append("------------");
	QString current_file = filename;
	QString currentFileName = current_file.right(current_file.length() - current_file.lastIndexOf('/') - 1);
	ui.logText->append("*"+currentFileName+"*");

	line = QString::asprintf("%.4f", DataCache::Instance().m_GGA2ZYZCache.baseXYH.dN);
	ui.logText->append(line);
	line = QString::asprintf("%.4f", DataCache::Instance().m_GGA2ZYZCache.baseXYH.dE);
	ui.logText->append(line);
	line = QString::asprintf("%.4f", DataCache::Instance().m_GGA2ZYZCache.baseXYH.dU);
	ui.logText->append(line);
	ui.logText->append("");

	line = QString::asprintf(u8"固定解个数：%d", DataCache::Instance().m_GGA2ZYZCache.fixedCount);
	ui.logText->append(line); message += line + "\n";
	line = QString::asprintf(u8"浮点解个数：%d", DataCache::Instance().m_GGA2ZYZCache.floatCount);
	ui.logText->append(line); message += line +"\n";
	line = QString::asprintf(u8"差分解个数：%d", DataCache::Instance().m_GGA2ZYZCache.dGPSCount);
	ui.logText->append(line); message += line + "\n";
	line = QString::asprintf(u8"单点解个数：%d", DataCache::Instance().m_GGA2ZYZCache.signalCount);
	ui.logText->append(line); message += line + "\n";
	line = QString::asprintf(u8"惯导解个数：%d", DataCache::Instance().m_GGA2ZYZCache.insCount);
	ui.logText->append(line); message += line + "\n";
	ui.logText->append("");

	QVector<int> fixedTimeList = DataCache::Instance().m_GGA2ZYZCache.fixedTimeList;
	for (int i = 0; i < fixedTimeList.size(); i++) {
		if (fixedTimeList[i] == -1) {
			line = u8"未固定";
		}
		else {
			line = QString::asprintf("%d", fixedTimeList[i]);
		}
		ui.logText->append(line);
	}
	createHDrift(currentFileName);
	createXYDrift(currentFileName);

	ThreadManager::Instance().m_ReadFileFlag = true;
	ThreadManager::Instance().m_ReadFileCondition.wakeAll();
	ThreadManager::Instance().m_ReadFileMutex.unlock();

	QMessageBox::information(this, u8"转换完毕", message);
}

void GGA2ZYZ::onClear()
{
	ui.logText->clear();
}

void GGA2ZYZ::createHDrift(QString filename)
{
#ifdef USE_QCP
	QCPHDriftChart* HDrift = nullptr;
#else
	HDriftChart* HDrift = nullptr;
#endif // USE_QCP
	if (m_HDriftMap.contains(filename)) {
        HDrift = m_HDriftMap.value(filename);
	}
	else {
#ifdef USE_QCP
        HDrift = new QCPHDriftChart();
#else
		HDrift = new HDriftChart();
#endif // USE_QCP
		m_HDriftMap.insert(filename, HDrift);
	}	
	QRect rect = QGuiApplication::screenAt(QCursor().pos())->availableGeometry();
	HDrift->move((rect.width() / 2 - HDrift->width()) / 2, rect.height() / 2 - HDrift->height());
	HDrift->updateHValues();
	HDrift->show();
	HDrift->activateWindow();
}

void GGA2ZYZ::createXYDrift(QString filename)
{
#ifdef USE_QCP
	QCPXYDriftChart* XYDrift = nullptr;
#else
	XYDriftChart* XYDrift = nullptr;
#endif // USE_QCP
	if (m_XYDriftMap.contains(filename)) {
		XYDrift = m_XYDriftMap.value(filename);
	}
	else {
#ifdef USE_QCP
        XYDrift = new QCPXYDriftChart();
#else
		XYDrift = new XYDriftChart();
#endif // USE_QCP
        m_XYDriftMap.insert(filename, XYDrift);
	}
	QRect rect = QGuiApplication::screenAt(QCursor().pos())->availableGeometry();
	XYDrift->move((rect.width() / 2 - XYDrift->width()) / 2, rect.height()/2 + 40);
	XYDrift->updateXYValues();
	XYDrift->show();
	XYDrift->activateWindow();
}

void GGA2ZYZ::closeAllChidren()
{
	for (auto it = m_HDriftMap.begin(); it != m_HDriftMap.end(); it++) {
		it.value()->close();
		it.value()->deleteLater();
	}
	m_HDriftMap.clear();
	for (auto it = m_XYDriftMap.begin(); it != m_XYDriftMap.end(); it++) {
		it.value()->close();
		it.value()->deleteLater();
	}
	m_XYDriftMap.clear();
}
