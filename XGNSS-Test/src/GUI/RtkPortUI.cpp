#include "RtkPortUI.h"
#include <QJsonObject>
#include <QJsonArray>
#include "ThreadManager.h"
#include "helper/Utils.h"

#define Listen_TXT	u8"Listen"
#define Close_TXT	u8"Close"

RtkPortUI::RtkPortUI(QWidget *parent)
	: QWidget(parent)
	, m_DataSize(0)
{
	ui.setupUi(this);
	connect(ui.listen_btn, SIGNAL(clicked()), this, SLOT(onStartClicked()));
	connect(ThreadManager::Instance().m_RtkServerThread, SIGNAL(sgnEnable(bool)), this, SLOT(onEnable(bool)), Qt::QueuedConnection);
	connect(ThreadManager::Instance().m_RtkServerThread, SIGNAL(sgnDataSize(const int)), this, SLOT(onDataSize(const int)), Qt::QueuedConnection);
}

RtkPortUI::~RtkPortUI()
{}

void RtkPortUI::loadConfig(QJsonObject& json)
{
	//读取全部配置
	QJsonObject config = json["RtkPort"].toObject();
	if (config.isEmpty())
		return;
	//加载port_box配置
	QJsonObject port_json = config["port"].toObject();
	ui.port_box->loadJson(port_json);
	//...
}

void RtkPortUI::saveConfig(QJsonObject& json)
{
	QJsonObject config = json["RtkPort"].toObject();
	//保存port_box配置
	QJsonObject port_json;
	ui.port_box->saveJson(port_json);
	config.insert("port", port_json);
	//...
	//保存全部配置
	json.insert("RtkPort", config);
}

void RtkPortUI::onStartClicked()
{
	if (ThreadManager::Instance().m_RtkServerThread->isRunning())
	{
		ThreadManager::Instance().m_RtkServerThread->stop();
		m_DataSize = 0;
		ui.read_size->setText(QString::number(m_DataSize));
	}
	else {
		ui.port_box->addCurruntText(ui.port_box->currentText().trimmed());
		m_DataSize = 0;
		ui.read_size->setText(QString::number(m_DataSize));
		ThreadManager::Instance().m_RtkServerThread->setPort(ui.port_box->currentText().toInt());
		ThreadManager::Instance().m_RtkServerThread->start();
	}
}

void RtkPortUI::onEnable(bool enable)
{
	if (ThreadManager::Instance().m_RtkServerThread->isRunning()) {
		ui.listen_btn->setText(Close_TXT);
		ui.listen_btn->setCheckable(true);
		ui.listen_btn->setChecked(true);
	}
	else {
		ui.listen_btn->setText(Listen_TXT);
		ui.listen_btn->setChecked(false);
		ui.listen_btn->setCheckable(false);
	}
}

void RtkPortUI::onDataSize(const int len)
{
	m_DataSize += len;
	ui.read_size->setText(FormatBytes(m_DataSize));
}