#include "LogFilePath.h"
#include "LogManager.h"
#include "ThreadManager.h"
#include <QFileDialog>
#include <QDesktopServices>
#include <QFontDataBase>

LogFilePath::LogFilePath(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	//引入图形字体
	int fontId = QFontDatabase::addApplicationFont(":/XGNSSTest/res/fontawesome-webfont.ttf");
	QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
	QFont iconFont = QFont(fontName);
	ui.m_ButtonSelect->setFont(iconFont);
	ui.m_ButtonSelect->setText(QChar(0xf07c));
	ui.m_ButtonSelect->setToolTip(u8"Choose Directory");
	ui.m_ButtonOpen->setFont(iconFont);
	ui.m_ButtonOpen->setText(QChar(0xf09d));
	ui.m_ButtonOpen->setToolTip(u8"Open In Explorer");
	connect(ui.m_ButtonSelect, SIGNAL(clicked()), this, SLOT(onSelectPathClicked()));
	connect(ui.m_ButtonOpen, SIGNAL(clicked()), this, SLOT(onOpenFileClicked()));
	connect(ui.saveFile, SIGNAL(toggled(bool)), this, SLOT(onSaveFileChecked(bool)));
}

LogFilePath::~LogFilePath()
{}

void LogFilePath::saveConfig(QJsonObject & config)
{
	config.insert("logPath", ui.m_EditFilePath->text());
}

void LogFilePath::loadConfig(QJsonObject& config)
{
	ui.m_EditFilePath->setText(config["logPath"].toString());
	LogManager::getInstance()->setLogRoot(ui.m_EditFilePath->text());
}

void LogFilePath::onOpenFileClicked()
{
	QString path = LogManager::getInstance()->getLogPath();
	if (path.isEmpty()) {
		path = ui.m_EditFilePath->text();
	}
	if (path.isEmpty()) {
		path = QDir::currentPath();
	}
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void LogFilePath::onSelectPathClicked()
{
	QString cur_path = LogManager::getInstance()->getLogRoot();
	QString path = QFileDialog::getExistingDirectory(this, "Choose Directory", cur_path);
	if (path.length() == 0) {
		return;
	}
	LogManager::getInstance()->setLogRoot(path);
	ui.m_EditFilePath->setText(path);
}

void LogFilePath::onSaveFileChecked(bool checked)
{
	if (checked) {
		ui.fileName->setEnabled(false);
		ThreadManager::Instance().m_SerialThread->setLogFile(true);
		ThreadManager::Instance().m_TcpClientThread->setLogFile(true);
		if (!ui.fileName->text().isEmpty()) {
			ThreadManager::Instance().m_SerialThread->setFileName(ui.fileName->text());
			ThreadManager::Instance().m_TcpClientThread->setFileName(ui.fileName->text());
		}
		else {
			ThreadManager::Instance().m_SerialThread->setFileName("user");
			ThreadManager::Instance().m_TcpClientThread->setFileName("user");
		}
	}
	else {
		ui.fileName->setEnabled(true);
		ThreadManager::Instance().m_SerialThread->setLogFile(false);
		ThreadManager::Instance().m_TcpClientThread->setLogFile(false);
	}
}
