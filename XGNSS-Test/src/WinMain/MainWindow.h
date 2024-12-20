#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"
#include "DockManager.h"
#include <QWidgetAction>

class ConfigFile;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr, QString name = "", QString version = "");
	~MainWindow();
protected:
	virtual void closeEvent(QCloseEvent* e);
	void SaveEvent();
	void LoadEvent();
	void saveConfig(QString filename);
	void loadConfig(QString filename);
	void ui2Json();
	void json2ui();
	void initWinPos();
	void initWinPosMini();
	void initToolBar();
	void createPerspectiveUi();
	void defaultLayout();
	void simpleLayout();
	void saveLayout(const QString& fileName);
	void restoreLayout(const QString& fileName);
	void savePerspectives();
	void loadPerspectives();
private slots:
	void onAddPerspective();
	void onChangeLayout(const QString& name);
private:
	Ui::MainWindowClass ui;
	ConfigFile* m_pConfigFile;
	ads::CDockManager* m_DockManager;
	ads::CDockWidget* m_ConsoleDock;
	ads::CDockWidget* m_DataTableDock;
	ads::CDockWidget* m_StreamCombineDock;
	//ads::CDockWidget* m_SerialportDock;
	//ads::CDockWidget* m_TcpClientDock;
	//ads::CDockWidget* m_NtripClientDock;
	ads::CDockWidget* m_LogFilePathDock;
	ads::CDockWidget* m_TriggerCommandDock;
	ads::CDockWidget* m_BinMsgTableDock;
	ads::CDockWidget* m_StarTableDock;
	//ads::CDockWidget* m_HVChartDock;
	ads::CDockWidget* m_QCPHVChartDock;
	ads::CDockWidget* m_GGA2ZYZDock;

	QAction* SavePerspectiveAction = nullptr;
	QWidgetAction* PerspectiveListAction = nullptr;
	QComboBox* PerspectiveComboBox = nullptr;
};
