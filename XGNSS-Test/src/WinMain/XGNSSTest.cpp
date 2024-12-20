#include "XGNSSTest.h"
#include <QStyle>
#include <QDebug>
#include <QScreen>
#include <QFile>
#include <QMessageBox>
#include "ConfigFile.h"
#include "LogManager.h"
#include "ThreadManager.h"

#include "helper/icontitlewidget.h"
#include "ads/SectionWidget.h"
#include "ads/DropOverlay.h"

#include "AboutDialog.h"
#include "SerialportWidget.h"
#include "TcpClientWidget.h"
#include "ControlWidget.h"
#include "DataTableUI.h"
#include "RtkPortUI.h"
#include "NtripClient.h"
#include "BinMsgTable.h"
#include "StarTable.h"
#include "LogFilePath.h"
#include "TriggerCommand.h"
#include "plot/HVChart.h"
#include "GGA2ZYZ.h"

#define CONFIG_FILE "./config.json"

static void storeDataHelper(const QString& fname, const QByteArray& ba)
{
	QFile f(fname + QString(".dat"));
	if (f.open(QFile::WriteOnly))
	{
		f.write(ba);
		f.close();
	}
}

static QByteArray loadDataHelper(const QString& fname)
{
	QFile f(fname + QString(".dat"));
	if (f.open(QFile::ReadOnly))
	{
		QByteArray ba = f.readAll();
		f.close();
		return ba;
	}
	return QByteArray();
}

#define MENU_Stream				u8"Stream"
#define MENU_Operation			u8"Operation"
#define MENU_View				u8"View"
#define MENU_Layout				u8"Layout"
#define MENU_About				u8"About"

#define TITLE_Console			u8"Console"
#define TITLE_DataTable			u8"DataTable"
#define TITLE_SerialPort		u8"SerialPort"
#define TITLE_TcpClient			u8"TcpClient"
#define TITLE_RtkPort			u8"RtkPort"
#define TITLE_NtripClient		u8"NtripClient"
#define TITLE_BinMsgTable		u8"BinMsgTable"
#define TITLE_StarTable			u8"StarTable"
#define TITLE_LogFilePath		u8"LogFilePath"
#define TITLE_TriggerCommand	u8"TriggerCommand"
#define TITLE_SimpleLayout		u8"SimpleLayout"
#define TITLE_DefautLayout		u8"DefautLayout"
#define TITLE_About				u8"About"
#define TITLE_HVChart			u8"HVChart"
#define TITLE_GGA2ZYZ			u8"GGA2ZYZ"

XGNSSTest::XGNSSTest(QWidget *parent, QString name, QString version)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	// ADS - Create main container (ContainerWidget).
	_container = new ADS_NS::ContainerWidget();
	_sw = NULL;
	setCentralWidget(_container);
	//�����Ӵ���
	//���ͺ���ʾָ���
	mCommandUIPtr = wrapWidget(new ControlWidget(), TITLE_Console);
	//ͳ�ƺͳ������ݱ��
	mDataTableUIPtr = wrapWidget(new DataTableUI, TITLE_DataTable);
	//��������Ϣ���
	mBinMsgTableUIPtr = wrapWidget(new BinMsgTable, TITLE_BinMsgTable);
	//���Ǳ��
	mStarTableUIPtr = wrapWidget(new StarTable, TITLE_StarTable);
	//�������Ӵ���
	mSerialportUIPtr = wrapWidget(new SerialportWidget, TITLE_SerialPort);
	//tcp�ͻ������Ӵ���
	mTcpClientUIPtr = wrapWidget(new TcpClientWidget, TITLE_TcpClient);
	//rtk��ֶ˿ڴ���
	//mRtkPortUIPtr = wrapWidget(new RtkPortUI, TITLE_RtkPort);
	//Ntrip����
	mNtripClientPtr = wrapWidget(new NtripClient, TITLE_NtripClient);
	//ָ�������
	mTriggerCommandPtr = wrapWidget(new TriggerCommand, TITLE_TriggerCommand);
	//�����ļ�·������
	mLogFilePathUIPtr = wrapWidget(new LogFilePath, TITLE_LogFilePath);
	//�������������ͼ��
	mHVChartPtr = wrapWidget(new HVChart, TITLE_HVChart);
	//GGAתXYZ
	mGGA2ZYZPtr = wrapWidget(new GGA2ZYZ, TITLE_GGA2ZYZ);
	defaultLayout();
	//����
	m_AboutDialog = new AboutDialog(this, name, version);
	//ָ����ź�������������
	connect(mTriggerCommandPtr->contentWidget(), SIGNAL(sgnAutoCommand(const QString)), mCommandUIPtr->contentWidget(), SLOT(onSendCmd(const QString)));
	//�����ź���Ӧ
	connect(_container, &ADS_NS::ContainerWidget::activeTabChanged, this, &XGNSSTest::onActiveTabChanged);
	connect(_container, &ADS_NS::ContainerWidget::sectionContentVisibilityChanged, this, &XGNSSTest::onSectionContentVisibilityChanged);
    //�Ӵ��ڲ˵�
	//�������˵�
    QMenu* streamMenu = ui.menuBar->addMenu(MENU_Stream);
    connect(streamMenu->addAction(TITLE_SerialPort), &QAction::triggered, this, &XGNSSTest::onShowSerialport);
    connect(streamMenu->addAction(TITLE_TcpClient), &QAction::triggered, this, &XGNSSTest::onShowTcpClient);
    //connect(streamMenu->addAction(TITLE_RtkPort), &QAction::triggered, this, &XGNSSTest::onShowRtkPort);
    connect(streamMenu->addAction(TITLE_NtripClient), &QAction::triggered, this, &XGNSSTest::onShowNtripClient);
	//���ֲ˵�
	QMenu* layoutMenu = ui.menuBar->addMenu(MENU_Layout);
	connect(layoutMenu->addAction(TITLE_SimpleLayout), &QAction::triggered, this, &XGNSSTest::onSetSimpleLayout);
	connect(layoutMenu->addAction(TITLE_DefautLayout), &QAction::triggered, this, &XGNSSTest::onSetDefautLayout);
	//��ͼ�˵�
	QMenu* viewMenu = ui.menuBar->addMenu(MENU_View);
	connect(viewMenu->addAction(TITLE_DataTable), &QAction::triggered, this, &XGNSSTest::onShowDataTable);
	connect(viewMenu->addAction(TITLE_BinMsgTable), &QAction::triggered, this, &XGNSSTest::onShowBinMsgTable);
	connect(viewMenu->addAction(TITLE_StarTable), &QAction::triggered, this, &XGNSSTest::onShowStarTable);
	//�����˵�
	QMenu* operationMenu = ui.menuBar->addMenu(MENU_Operation);
	connect(operationMenu->addAction(TITLE_LogFilePath), &QAction::triggered, this, &XGNSSTest::onShowLogFilePath);
	connect(operationMenu->addAction(TITLE_Console), &QAction::triggered, this, &XGNSSTest::onShowCommandUI);
	connect(operationMenu->addAction(TITLE_GGA2ZYZ), &QAction::triggered, this, &XGNSSTest::onShowGGA2ZYZ);
	//About�˵�
	QMenu* menuAbout = ui.menuBar->addMenu(MENU_About);
	menuAbout->addAction(TITLE_About, this, SLOT(onAboutDialog()));
	//����λ��
	QRect rect = QGuiApplication::screens()[0]->availableGeometry();
	int w = rect.width() / 2;
	int h = rect.height() / 2;
	qDebug("window.w = %d,window.h=%d", w, h);
	rect.setRect(w/2,h/2,w+64,h);
	setGeometry(rect);
	//���������ļ�
	m_pConfigFile = new ConfigFile(this);
	LoadEvent();
	//��¼����λ�ô�С
	mWindowGeometry = saveGeometry();
	//��¼���ڲ���
	mWindowLayout = _container->saveState();
}

XGNSSTest::~XGNSSTest()
{
	delete m_pConfigFile;
	_container->removeSectionContent(mCommandUIPtr);
	_container->removeSectionContent(mDataTableUIPtr);
	_container->removeSectionContent(mSerialportUIPtr);
	_container->removeSectionContent(mTcpClientUIPtr);
	//_container->removeSectionContent(mRtkPortUIPtr);
	_container->removeSectionContent(mNtripClientPtr);
	_container->removeSectionContent(mBinMsgTableUIPtr);
	_container->removeSectionContent(mStarTableUIPtr);
	_container->removeSectionContent(mTriggerCommandPtr);
	_container->removeSectionContent(mLogFilePathUIPtr);
	_container->removeSectionContent(mHVChartPtr);
	_container->removeSectionContent(mGGA2ZYZPtr);
}

ADS_NS::SectionContent::RefPtr XGNSSTest::wrapWidget(QWidget* w, QString name)
{
	return ADS_NS::SectionContent::newSectionContent(name, _container, new IconTitleWidget(QIcon(), name), w);
}

void XGNSSTest::defaultLayout()
{
	//���ͺ���ʾָ���
	ADS_NS::SectionWidget* commandPos = _container->addSectionContent(mCommandUIPtr, NULL, ADS_NS::CenterDropArea);
	//��������Ϣ���
	_container->addSectionContent(mBinMsgTableUIPtr, commandPos, ADS_NS::CenterDropArea);
	//���Ǳ��
	_container->addSectionContent(mStarTableUIPtr, commandPos, ADS_NS::CenterDropArea);
	//�������������ͼ��
	_container->addSectionContent(mHVChartPtr, commandPos, ADS_NS::CenterDropArea);
	//GGAתXYZ
	_container->addSectionContent(mGGA2ZYZPtr, commandPos, ADS_NS::CenterDropArea);
	//ͳ�ƺͳ������ݱ��
	ADS_NS::SectionWidget* dataTablePos = _container->addSectionContent(mDataTableUIPtr, commandPos, ADS_NS::LeftDropArea);
	//Ntrip����
	ADS_NS::SectionWidget* NtripPos = _container->addSectionContent(mNtripClientPtr, dataTablePos, ADS_NS::LeftDropArea);
	//�������Ӵ���
	_sw = _container->addSectionContent(mSerialportUIPtr, NtripPos, ADS_NS::TopDropArea);
	//tcp�ͻ������Ӵ���
	_sw = _container->addSectionContent(mTcpClientUIPtr, _sw, ADS_NS::CenterDropArea);
	//rtk��ֶ˿ڴ���
	//_sw = _container->addSectionContent(mRtkPortUIPtr, _sw, ADS_NS::CenterDropArea);
	//ָ�������
	_container->addSectionContent(mTriggerCommandPtr, NtripPos, ADS_NS::BottomDropArea);

	//�����ļ�·������
	_container->addSectionContent(mLogFilePathUIPtr, commandPos, ADS_NS::TopDropArea);
}

void XGNSSTest::SaveEvent()
{
	ControlWidget* w = qobject_cast<ControlWidget*>(mCommandUIPtr->contentWidget());
	if (w) {
		w->saveCommands();
	}
	saveConfig(CONFIG_FILE);
#ifdef SAVE_LAYOUT
	storeDataHelper("MainWindow", saveGeometry());
	storeDataHelper("ContainerWidget", _container->saveState());
#endif // SAVE_LAYOUT
}

void XGNSSTest::LoadEvent()
{
#ifdef SAVE_LAYOUT
	restoreGeometry(loadDataHelper("MainWindow"));
	_container->restoreState(loadDataHelper("ContainerWidget"));
#endif // SAVE_LAYOUT
	loadConfig(CONFIG_FILE);
	ControlWidget* w = qobject_cast<ControlWidget*>(mCommandUIPtr->contentWidget());
	if (w) {
		w->loadCommands();
	}
}

void XGNSSTest::saveConfig(QString filename)
{
	ui2Json();
	m_pConfigFile->writeConfigFile(filename);
	ControlWidget* w = qobject_cast<ControlWidget*>(mCommandUIPtr->contentWidget());
	if(w) w->onShowMsg(MSG_INFO, "Save config:" + filename);
}

void XGNSSTest::loadConfig(QString filename)
{
	ControlWidget* w = qobject_cast<ControlWidget*>(mCommandUIPtr->contentWidget());
	if (w) w->onShowMsg(MSG_INFO, "Load config:" + filename);
	if (!m_pConfigFile->readConfigFile(filename)) {
		return;
	}
	json2ui();
}

void XGNSSTest::ui2Json()
{
	qobject_cast<SerialportWidget*>(mSerialportUIPtr->contentWidget())->saveConfig(m_pConfigFile->m_ConfigJson);
	qobject_cast<TcpClientWidget*>(mTcpClientUIPtr->contentWidget())->saveConfig(m_pConfigFile->m_ConfigJson);
	//qobject_cast<RtkPortUI*>(mRtkPortUIPtr->contentWidget())->saveConfig(m_pConfigFile->m_ConfigJson);
	qobject_cast<NtripClient*>(mNtripClientPtr->contentWidget())->saveConfig(m_pConfigFile->m_ConfigJson);
	qobject_cast<LogFilePath*>(mLogFilePathUIPtr->contentWidget())->saveConfig(m_pConfigFile->m_ConfigJson);
}

void XGNSSTest::json2ui()
{
	qobject_cast<SerialportWidget*>(mSerialportUIPtr->contentWidget())->loadConfig(m_pConfigFile->m_ConfigJson);
	qobject_cast<TcpClientWidget*>(mTcpClientUIPtr->contentWidget())->loadConfig(m_pConfigFile->m_ConfigJson);
	//qobject_cast<RtkPortUI*>(mRtkPortUIPtr->contentWidget())->loadConfig(m_pConfigFile->m_ConfigJson);
	qobject_cast<NtripClient*>(mNtripClientPtr->contentWidget())->loadConfig(m_pConfigFile->m_ConfigJson);
	qobject_cast<LogFilePath*>(mLogFilePathUIPtr->contentWidget())->loadConfig(m_pConfigFile->m_ConfigJson);
}

void XGNSSTest::onSetSimpleLayout()
{
	//�ı䴰�ڴ�С
	QRect rect = geometry();
	//�ر�
	_container->hideSectionContent(mCommandUIPtr);
	_container->hideSectionContent(mDataTableUIPtr);
	_container->hideSectionContent(mSerialportUIPtr);
	_container->hideSectionContent(mTcpClientUIPtr);
	//_container->hideSectionContent(mRtkPortUIPtr);
	_container->hideSectionContent(mNtripClientPtr);
	_container->hideSectionContent(mBinMsgTableUIPtr);
	_container->hideSectionContent(mStarTableUIPtr);
	_container->hideSectionContent(mHVChartPtr);
	_container->hideSectionContent(mGGA2ZYZPtr);
	_container->hideSectionContent(mTriggerCommandPtr);
	_container->hideSectionContent(mLogFilePathUIPtr);
	//����
	_container->showSectionContent(mDataTableUIPtr);
	//�ı䴰�ڴ�С
	rect.setRect(rect.x(), rect.y(), 300, 780);
	setGeometry(rect);
}

void XGNSSTest::onSetDefautLayout()
{
	restoreGeometry(mWindowGeometry);
	_container->restoreState(mWindowLayout);
}

void XGNSSTest::onShowSerialport()
{
	if (_container->isSectionContentVisible(mSerialportUIPtr)) {
		_container->hideSectionContent(mSerialportUIPtr);
	}
	else {
		_container->showSectionContent(mSerialportUIPtr);
	}
}

void XGNSSTest::onShowTcpClient()
{
	if (_container->isSectionContentVisible(mTcpClientUIPtr)) {
		_container->hideSectionContent(mTcpClientUIPtr);
	}
	else {
		_container->showSectionContent(mTcpClientUIPtr);
	}
}

//void XGNSSTest::onShowRtkPort()
//{
//	if (_container->isSectionContentVisible(mRtkPortUIPtr)) {
//		_container->hideSectionContent(mRtkPortUIPtr);
//	}
//	else {
//		_container->showSectionContent(mRtkPortUIPtr);
//	}
//}

void XGNSSTest::onShowNtripClient()
{
	if (_container->isSectionContentVisible(mNtripClientPtr)) {
		_container->hideSectionContent(mNtripClientPtr);
	}
	else {
		_container->showSectionContent(mNtripClientPtr);
	}
}

void XGNSSTest::onShowCommandUI()
{
	if (_container->isSectionContentVisible(mCommandUIPtr)) {
		_container->hideSectionContent(mCommandUIPtr);
	}
	else {
		_container->showSectionContent(mCommandUIPtr);
	}
}

void XGNSSTest::onShowDataTable()
{
	if (_container->isSectionContentVisible(mDataTableUIPtr)) {
		_container->hideSectionContent(mDataTableUIPtr);
	}
	else {
		_container->showSectionContent(mDataTableUIPtr);
	}
}

void XGNSSTest::onShowBinMsgTable()
{
	if (_container->isSectionContentVisible(mBinMsgTableUIPtr)) {
		_container->hideSectionContent(mBinMsgTableUIPtr);
	}
	else {
		_container->showSectionContent(mBinMsgTableUIPtr);
	}
}

void XGNSSTest::onShowStarTable()
{
	if (_container->isSectionContentVisible(mStarTableUIPtr)) {
		_container->hideSectionContent(mStarTableUIPtr);
	}
	else {
		_container->showSectionContent(mStarTableUIPtr);
	}
}

void XGNSSTest::onShowLogFilePath()
{
	if (_container->isSectionContentVisible(mLogFilePathUIPtr)) {
		_container->hideSectionContent(mLogFilePathUIPtr);
	}
	else {
		_container->showSectionContent(mLogFilePathUIPtr);
	}
}

void XGNSSTest::onShowGGA2ZYZ()
{
	if (_container->isSectionContentVisible(mGGA2ZYZPtr)) {
		_container->hideSectionContent(mGGA2ZYZPtr);
	}
	else {
		_container->showSectionContent(mGGA2ZYZPtr);
	}
}

void XGNSSTest::onAboutDialog()
{
	m_AboutDialog->exec();
}

void XGNSSTest::onActiveTabChanged(const ADS_NS::SectionContent::RefPtr& sc, bool active)
{
	Q_UNUSED(active);
	IconTitleWidget* itw = dynamic_cast<IconTitleWidget*>(sc->titleWidget());
	if (itw)
	{
		itw->polishUpdate();
	}
}

void XGNSSTest::onSectionContentVisibilityChanged(const ADS_NS::SectionContent::RefPtr& sc, bool visible)
{
	qDebug() << Q_FUNC_INFO << sc->uniqueName() << visible;
}

void XGNSSTest::closeEvent(QCloseEvent* event)
{
	if (ThreadManager::Instance().m_SerialThread->isRunning() || ThreadManager::Instance().m_TcpClientThread->isRunning()) {
		QMessageBox::warning(this, u8"����", u8"���ȹرն˿ڡ�");
		event->ignore();
		return;
	}
	//Q_UNUSED(e);
	// ����ȷ�϶Ի���
	QMessageBox::StandardButton button = QMessageBox::question(
		this, u8"�˳�����", u8"��ȷ��Ҫ�˳���",
		QMessageBox::Yes | QMessageBox::No);

	// �����û�ѡ�����¼�
	if (button == QMessageBox::Yes) {
		// �û�ѡ��"��"���رմ���
		SaveEvent();
		event->accept();
	}
	else {
		// �û�ѡ��"��"��ȡ���ر�
		event->ignore();
	}
}