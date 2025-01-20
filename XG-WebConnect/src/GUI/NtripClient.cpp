#include "NtripClient.h"
#include "ThreadManager.h"
#include <QJsonArray>
#include "helper/Utils.h"

NtripClient::NtripClient(QWidget *parent)
	: QWidget(parent)
	, m_DataSize(0)
{
	ui.setupUi(this);
	ui.saveFile->hide();
	connect(ui.connect_btn, SIGNAL(clicked()),this,SLOT(onConnectClicked()));

	connect(ThreadManager::Instance().m_NtripThread, SIGNAL(sgnEnable(bool)),this,SLOT(onEnable(bool)), Qt::QueuedConnection);
	//connect(ThreadManager::Instance().m_NtripThread, SIGNAL(sgnStream(const QByteArray)),this,SLOT(onStream(const QByteArray)));
	connect(ThreadManager::Instance().m_NtripThread, SIGNAL(sgnDataSize(const int)),this,SLOT(onDataSize(const int)), Qt::QueuedConnection);
	connect(ui.saveFile, SIGNAL(toggled(bool)), this, SLOT(onSaveFileChecked(bool)));
}

NtripClient::~NtripClient()
{}

void NtripClient::loadConfig(QJsonObject & json)
{
	//读取全部配置
	QJsonObject config = json["Ntrip"].toObject();
	if (config.isEmpty())
		return;
	//加载配置
	QJsonObject host_json = config["host"].toObject();
	ui.host_box->loadJson(host_json);
	QJsonObject port_json = config["port"].toObject();
	ui.port_box->loadJson(port_json);
	QJsonObject mount_json = config["mount"].toObject();
	ui.mount_box->loadJson(mount_json);
	QJsonObject username_json = config["username"].toObject();
	ui.username_box->loadJson(username_json);
	//QJsonObject password_json = config["password"].toObject();
	//ui.password_box->loadJson(password_json);
	//...
}

void NtripClient::saveConfig(QJsonObject& json)
{
	QJsonObject config = json["Ntrip"].toObject();
	//保存配置
	QJsonObject host_json;
	ui.host_box->saveJson(host_json);
	config.insert("host", host_json);
	QJsonObject port_json;
	ui.port_box->saveJson(port_json);
	config.insert("port", port_json);
	QJsonObject mount_json;
	ui.mount_box->saveJson(mount_json);
	config.insert("mount", mount_json);
	QJsonObject username_json;
	ui.username_box->saveJson(username_json);
	config.insert("username", username_json);
	//QJsonObject password_json;
	//ui.password_box->saveJson(password_json);
	//config.insert("password", password_json);
	//...
	//保存全部配置
	json.insert("Ntrip", config);	
}

void NtripClient::onConnectClicked()
{
	if (ThreadManager::Instance().m_NtripThread->isRunning()) {
		ThreadManager::Instance().m_NtripThread->stop();
	}
	else {
		m_DataSize = 0;
		ThreadManager::Instance().m_NtripThread->setNtripParams(ui.host_box->currentText(), ui.port_box->currentText().toInt(),
			ui.mount_box->currentText(), ui.username_box->currentText(), ui.password->text());
		ThreadManager::Instance().m_NtripThread->setLogFile(ui.saveFile->isChecked());
		ThreadManager::Instance().m_NtripThread->start();
	}
}

void NtripClient::onEnable(bool enable)
{
	enable = !ThreadManager::Instance().m_NtripThread->isRunning();
	ui.host_box->setEnabled(enable);
	ui.port_box->setEnabled(enable);
	ui.mount_box->setEnabled(enable);
	ui.username_box->setEnabled(enable);
	ui.password->setEnabled(enable);
	ui.connect_btn->setCheckable(!enable);
	ui.connect_btn->setChecked(!enable);
	if (enable) {
		ui.connect_btn->setText("Connect");
	}
	else {
		ui.connect_btn->setText("Disconnect");
	}
}

void NtripClient::onDataSize(const int len)
{
	m_DataSize += len;
	ui.read_size_line->setText(FormatBytes(m_DataSize));
}

void NtripClient::onSaveFileChecked(bool checked)
{
	if (checked) {
        ThreadManager::Instance().m_NtripThread->setLogFile(true);
	}
	else {
		ThreadManager::Instance().m_NtripThread->setLogFile(false);
    }
}
