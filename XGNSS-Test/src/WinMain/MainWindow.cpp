#include "MainWindow.h"
#include <QScreen>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QInputDialog>
#include "ConfigFile.h"
#include "LogManager.h"
#include "ThreadManager.h"

#include "StreamCombine.h"
#include "SerialportWidget.h"
#include "TcpClientWidget.h"
#include "NtripClient.h"
#include "ControlWidget.h"
#include "DataTableUI.h"
#include "TriggerCommand.h"
#include "LogFilePath.h"
#include "BinMsgTable.h"
#include "StarTable.h"
#include "plot/HVChart.h"
#include "QCPlot/QCPHVChart.h"
#include "GGA2ZYZ.h"

//#define SAVE_LAYOUT

#define CONFIG_FILE "./config.json"

#define MENU_Stream				u8"Stream"
#define MENU_Operation			u8"Operation"
#define MENU_View				u8"View"
#define MENU_Tools				u8"Tools"
#define MENU_Layout				u8"Layout"
#define MENU_About				u8"About"

#define TITLE_DataTable			u8"DataTable"
#define TITLE_Console			u8"Console"
#define TITLE_StreamCombine		u8"Streams"
#define TITLE_SerialPort		u8"SerialPort"
#define TITLE_TcpClient			u8"TcpClient"
#define TITLE_NtripClient		u8"NtripClient"
#define TITLE_BinMsgTable		u8"BinMsgTable"
#define TITLE_StarTable			u8"StarTable"
#define TITLE_LogFilePath		u8"LogFilePath"
#define TITLE_TriggerCommand	u8"TriggerCommand"
#define TITLE_SimpleLayout		u8"SimpleLayout"
#define TITLE_DefautLayout		u8"DefautLayout"
#define TITLE_About				u8"About"
#define TITLE_HVChart			u8"HVChart"
#define TITLE_QCPHVChart		u8"QCPHVChart"
#define TITLE_GGA2ZYZ			u8"GGA2ZYZ"

MainWindow::MainWindow(QWidget *parent, QString name, QString version)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowTitle(name + " " + version);
	//ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
	ads::CDockManager::setCustomStyle(":/XGNSSTest/res/style-dock.css");
	m_DockManager = new ads::CDockManager(this);
	m_ConsoleDock = new ads::CDockWidget(TITLE_Console);
	m_ConsoleDock->setWidget(new ControlWidget());
	m_DataTableDock = new ads::CDockWidget(TITLE_DataTable);
    m_DataTableDock->setWidget(new DataTableUI());
	m_StreamCombineDock = new ads::CDockWidget(TITLE_StreamCombine);
    m_StreamCombineDock->setWidget(new StreamCombine());
	//m_SerialportDock = new ads::CDockWidget(TITLE_SerialPort);
	//m_SerialportDock->setWidget(new SerialportWidget());
	//m_TcpClientDock = new ads::CDockWidget(TITLE_TcpClient);
	//m_TcpClientDock->setWidget(new TcpClientWidget());
	//m_NtripClientDock = new ads::CDockWidget(TITLE_NtripClient);
	//m_NtripClientDock->setWidget(new NtripClient());
	m_LogFilePathDock = new ads::CDockWidget(TITLE_LogFilePath);
    m_LogFilePathDock->setWidget(new LogFilePath());
	m_TriggerCommandDock = new ads::CDockWidget(TITLE_TriggerCommand);
	m_TriggerCommandDock->setWidget(new TriggerCommand());
	m_BinMsgTableDock = new ads::CDockWidget(TITLE_BinMsgTable);
	m_BinMsgTableDock->setWidget(new BinMsgTable());
	m_StarTableDock = new ads::CDockWidget(TITLE_StarTable);
	m_StarTableDock->setWidget(new StarTable());
	//m_HVChartDock = new ads::CDockWidget(TITLE_HVChart);
 //   m_HVChartDock->setWidget(new HVChart());
	m_QCPHVChartDock = new ads::CDockWidget(TITLE_HVChart);
	m_QCPHVChartDock->setWidget(new QCPHVChart());
	m_GGA2ZYZDock = new ads::CDockWidget(TITLE_GGA2ZYZ);
	m_GGA2ZYZDock->setWidget(new GGA2ZYZ());

	StreamCombine* streamCombine = qobject_cast<StreamCombine*>(m_StreamCombineDock->widget());
	DataTableUI* dataTable = qobject_cast<DataTableUI*>(m_DataTableDock->widget());
	connect(dataTable->mStreamEasy, SIGNAL(sgnStartSerialport()), streamCombine->serialPortWidget, SLOT(onStartClicked()));
	connect(dataTable->mStreamEasy, SIGNAL(sgnStartTcpClient()), streamCombine->tcpClientWidget, SLOT(onStartClicked()));
	connect(dataTable->mStreamEasy, SIGNAL(sgnStartNtrip()), streamCombine->ntripClient, SLOT(onConnectClicked()));
	connect(dataTable->mStreamEasy, SIGNAL(sgnSettingToggled(bool)), m_StreamCombineDock, SLOT(toggleView(bool)));
	//dataTable->mStreamEasy->setSettingAction(m_StreamCombineDock->toggleViewAction());
	//数据流菜单
	//QMenu* streamMenu = ui.menuBar->addMenu(MENU_Stream);
	//streamMenu->addAction(m_StreamCombineDock->toggleViewAction());
	//streamMenu->addAction(m_SerialportDock->toggleViewAction());
	//streamMenu->addAction(m_TcpClientDock->toggleViewAction());
	//streamMenu->addAction(m_NtripClientDock->toggleViewAction());
	//操作菜单
	QMenu* operationMenu = ui.menuBar->addMenu(MENU_Operation);
	operationMenu->addAction(m_StreamCombineDock->toggleViewAction());
	operationMenu->addAction(m_ConsoleDock->toggleViewAction());
	operationMenu->addAction(m_LogFilePathDock->toggleViewAction());
	operationMenu->addAction(m_TriggerCommandDock->toggleViewAction());
	//视图菜单
	QMenu* viewMenu = ui.menuBar->addMenu(MENU_View);
	viewMenu->addAction(m_BinMsgTableDock->toggleViewAction());
    viewMenu->addAction(m_StarTableDock->toggleViewAction());
    //viewMenu->addAction(m_HVChartDock->toggleViewAction());
    viewMenu->addAction(m_QCPHVChartDock->toggleViewAction());
	//工具菜单
	QMenu* toolsMenu = ui.menuBar->addMenu(MENU_Tools);
    toolsMenu->addAction(m_GGA2ZYZDock->toggleViewAction());
	//窗口位置
	//initWinPos();
	initWinPosMini();
	//默认布局
	//defaultLayout();
	simpleLayout();
	//初始化工具栏
	initToolBar();
	//加载文件
	m_pConfigFile = new ConfigFile(this);
	LoadEvent();
}

MainWindow::~MainWindow()
{
	delete m_pConfigFile;
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	if (ThreadManager::Instance().m_SerialThread->isRunning() || ThreadManager::Instance().m_TcpClientThread->isRunning()) {
		QMessageBox::warning(this, u8"警告", u8"请先关闭端口。");
		event->ignore();
		return;
	}
	//Q_UNUSED(e);
	// 弹出确认对话框
	QMessageBox::StandardButton button = QMessageBox::question(
		this, u8"退出程序", u8"你确定要退出吗？",
		QMessageBox::Yes | QMessageBox::No);

	// 根据用户选择处理事件
	if (button == QMessageBox::Yes) {
		// 用户选择"是"，关闭窗口
		SaveEvent();
		event->accept();
		m_DockManager->deleteLater();
	}
	else {
		// 用户选择"否"，取消关闭
		event->ignore();
	}
}

void MainWindow::SaveEvent()
{
	ControlWidget* w = qobject_cast<ControlWidget*>(m_ConsoleDock->widget());
	if (w) {
		w->saveCommands();
	}
	saveConfig(CONFIG_FILE);
#ifdef SAVE_LAYOUT
    saveLayout("MiniLayout.ini");
	//saveLayout("NormalLayout.ini");
#endif // SAVE_LAYOUT
	//savePerspectives();
}

void MainWindow::LoadEvent()
{
	//loadPerspectives();
	onChangeLayout("MiniLayout");
	loadConfig(CONFIG_FILE);
	ControlWidget* w = qobject_cast<ControlWidget*>(m_ConsoleDock->widget());
	if (w) {
		w->loadCommands();
	}
}

void MainWindow::saveConfig(QString filename)
{
	ui2Json();
	m_pConfigFile->writeConfigFile(filename);
	ControlWidget* w = qobject_cast<ControlWidget*>(m_ConsoleDock->widget());
	if (w) w->onShowMsg(MSG_INFO, "Save config:" + filename);
}

void MainWindow::loadConfig(QString filename)
{
	ControlWidget* w = qobject_cast<ControlWidget*>(m_ConsoleDock->widget());
	if (w) w->onShowMsg(MSG_INFO, "Load config:" + filename);
	if (!m_pConfigFile->readConfigFile(filename)) {
		return;
	}
	json2ui();
}

void MainWindow::ui2Json()
{
	qobject_cast<StreamCombine*>(m_StreamCombineDock->widget())->saveConfig(m_pConfigFile->m_ConfigJson);
	qobject_cast<LogFilePath*>(m_LogFilePathDock->widget())->saveConfig(m_pConfigFile->m_ConfigJson);
}

void MainWindow::json2ui()
{
	qobject_cast<StreamCombine*>(m_StreamCombineDock->widget())->loadConfig(m_pConfigFile->m_ConfigJson);
	qobject_cast<LogFilePath*>(m_LogFilePathDock->widget())->loadConfig(m_pConfigFile->m_ConfigJson);
}

void MainWindow::initWinPos()
{
	QRect rect = QGuiApplication::screens()[0]->availableGeometry();
	int w = 1024;// rect.width() * 45 / 100;
	int h = 768;//rect.height() * 60/100;
	qDebug("window.w = %d,window.h=%d", w, h);
	rect.setRect((rect.width()-w) / 2, (rect.height()-h) / 2, w + 64, h);
	setGeometry(rect);
}

void MainWindow::initWinPosMini()
{
	QRect rect = QGuiApplication::screens()[0]->availableGeometry();
	int w = 378;// rect.width() * 45 / 100;
	int h = 500;//rect.height() * 60/100;
	qDebug("window.w = %d,window.h=%d", w, h);
	rect.setRect((rect.width() - w) / 2, (rect.height() - h) / 2, w + 64, h);
	setGeometry(rect);
}

void MainWindow::initToolBar()
{
	createPerspectiveUi();
}

void MainWindow::createPerspectiveUi()
{
	PerspectiveListAction = new QWidgetAction(this);
	PerspectiveComboBox = new QComboBox(this);
	PerspectiveComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	PerspectiveComboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	PerspectiveComboBox->addItem("MiniLayout");
    PerspectiveComboBox->addItem("NormalLayout");
	connect(PerspectiveComboBox, SIGNAL(currentTextChanged(const QString&)),
		this, SLOT(onChangeLayout(const QString&)));
	PerspectiveListAction->setDefaultWidget(PerspectiveComboBox);
	ui.toolBar->addSeparator();
	ui.toolBar->addAction(PerspectiveListAction);
	//SavePerspectiveAction = new QAction("Create", this);
	//connect(SavePerspectiveAction, SIGNAL(triggered()), SLOT(onAddPerspective()));
	//ui.toolBar->addAction(SavePerspectiveAction);
}

void MainWindow::defaultLayout()
{
	auto dataTableArea = m_DockManager->addDockWidget(ads::CenterDockWidgetArea,m_DataTableDock);
	auto streamArea = m_DockManager->addDockWidget(ads::LeftDockWidgetArea, m_StreamCombineDock);
	QList<int> consoleSp = m_DockManager->splitterSizes(dataTableArea);
	if (consoleSp.size() == 2) {
		consoleSp[0] = 220;
		m_DockManager->setSplitterSizes(dataTableArea, consoleSp);
	}

	auto logpathArea = m_DockManager->addDockWidget(ads::TopDockWidgetArea, m_LogFilePathDock, dataTableArea);
	m_DockManager->addDockWidgetTabToArea(m_ConsoleDock, dataTableArea);
	m_DataTableDock->setAsCurrentTab();
	QList<int> logpathSp = m_DockManager->splitterSizes(logpathArea);
	if (logpathSp.size() == 2) {
		logpathSp[0] = 0;
		logpathSp[1] = 240;
		m_DockManager->setSplitterSizes(logpathArea, logpathSp);
	}
	m_DockManager->addDockWidgetFloating(m_BinMsgTableDock);
	m_BinMsgTableDock->toggleView(false);
	m_DockManager->addDockWidgetFloating(m_StarTableDock);
	m_StarTableDock->toggleView(false);
	//m_DockManager->addDockWidgetFloating(m_HVChartDock);
	//m_HVChartDock->toggleView(false);	
	m_DockManager->addDockWidgetFloating(m_QCPHVChartDock);
	m_QCPHVChartDock->toggleView(false);
	m_DockManager->addDockWidgetFloating(m_TriggerCommandDock);
	m_TriggerCommandDock->toggleView(false);
	m_DockManager->addDockWidgetFloating(m_GGA2ZYZDock);
	m_GGA2ZYZDock->toggleView(false);
}

void MainWindow::simpleLayout()
{
	m_DockManager->addDockWidget(ads::CenterDockWidgetArea, m_DataTableDock);

	m_DockManager->addDockWidgetFloating(m_StreamCombineDock);
	m_StreamCombineDock->toggleView(false);
	m_DockManager->addDockWidgetFloating(m_LogFilePathDock);
	m_LogFilePathDock->toggleView(false);
	m_DockManager->addDockWidgetFloating(m_ConsoleDock);
	m_ConsoleDock->toggleView(false);
	m_DockManager->addDockWidgetFloating(m_BinMsgTableDock);
	m_BinMsgTableDock->toggleView(false);
	m_DockManager->addDockWidgetFloating(m_StarTableDock);
	m_StarTableDock->toggleView(false);
	//m_DockManager->addDockWidgetFloating(m_HVChartDock);
	//m_HVChartDock->toggleView(false);
	m_DockManager->addDockWidgetFloating(m_QCPHVChartDock);
	m_QCPHVChartDock->toggleView(false);
	m_DockManager->addDockWidgetFloating(m_TriggerCommandDock);
	m_TriggerCommandDock->toggleView(false);
	m_DockManager->addDockWidgetFloating(m_GGA2ZYZDock);
	m_GGA2ZYZDock->toggleView(false);
}

void MainWindow::saveLayout(const QString& fileName)
{
	QSettings Settings(fileName, QSettings::IniFormat);
	Settings.setValue("mainWindow/Geometry", this->saveGeometry());
	Settings.setValue("mainWindow/State", this->saveState());
	Settings.setValue("mainWindow/DockingState", m_DockManager->saveState());
}

void MainWindow::restoreLayout(const QString& fileName)
{
	QSettings Settings(fileName, QSettings::IniFormat);
	this->restoreGeometry(Settings.value("mainWindow/Geometry").toByteArray());
	this->restoreState(Settings.value("mainWindow/State").toByteArray());
	m_DockManager->restoreState(Settings.value("mainWindow/DockingState").toByteArray());
}

void MainWindow::savePerspectives()
{
	QSettings Settings("Perspectives.ini", QSettings::IniFormat);
	m_DockManager->savePerspectives(Settings);
}

void MainWindow::loadPerspectives()
{
	QSettings Settings("Perspectives.ini", QSettings::IniFormat);
	m_DockManager->loadPerspectives(Settings);
	QSignalBlocker Blocker(PerspectiveComboBox);
	PerspectiveComboBox->clear();
	for (int i = 0; i < m_DockManager->perspectiveNames().size(); i++) {
        PerspectiveComboBox->addItem(m_DockManager->perspectiveNames().at(i));
	}
}

void MainWindow::onChangeLayout(const QString& name)
{
	if (name == "MiniLayout") {
		restoreLayout(":/XGNSSTest/res/MiniLayout.ini");
	}
	else if (name == "NormalLayout") {
        restoreLayout(":/XGNSSTest/res/NormalLayout.ini");
	}
}

void MainWindow::onAddPerspective()
{
	QString PerspectiveName = QInputDialog::getText(this, "Save Perspective", "Enter unique name:");
	if (PerspectiveName.isEmpty())
	{
		return;
	}
	m_DockManager->addPerspective(PerspectiveName);
	QSignalBlocker Blocker(PerspectiveComboBox);
	PerspectiveComboBox->clear();
	PerspectiveComboBox->addItems(m_DockManager->perspectiveNames());
	PerspectiveComboBox->setCurrentText(PerspectiveName);
}
