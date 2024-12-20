#include "WebSocketUI.h"
#include "ThreadManager.h"
#include "helper/Utils.h"

#define Listen_TXT	u8"Listen"
#define Close_TXT	u8"Close"

WebSocketUI::WebSocketUI(QWidget *parent)
	: QWidget(parent)
	, m_DataSize(0)
{
	ui.setupUi(this);
	ui.port_box->addCurruntText("8888");
	connect(ui.listen_btn, SIGNAL(clicked()), this, SLOT(onStartClicked()));
	connect(ThreadManager::Instance().m_WebSocketServerThread, SIGNAL(sgnEnable(bool)), this, SLOT(onEnable(bool)), Qt::QueuedConnection);
	connect(ThreadManager::Instance().m_WebSocketServerThread, SIGNAL(sgnDataSize(const int)), this, SLOT(onDataSize(const int)), Qt::QueuedConnection);

	//Ä¬ÈÏÆô¶¯
	onStartClicked();
}

WebSocketUI::~WebSocketUI()
{}

void WebSocketUI::onStartClicked()
{
	if (ThreadManager::Instance().m_WebSocketServerThread->isRunning())
	{
		ThreadManager::Instance().m_WebSocketServerThread->stop();
		m_DataSize = 0;
		ui.send_size->setText(QString::number(m_DataSize));
	}
	else {
		ui.port_box->addCurruntText(ui.port_box->currentText().trimmed());
		m_DataSize = 0;
		ui.send_size->setText(QString::number(m_DataSize));
		ThreadManager::Instance().m_WebSocketServerThread->setPort(ui.port_box->currentText().toInt());
		ThreadManager::Instance().m_WebSocketServerThread->start();
	}
}

void WebSocketUI::onEnable(bool enable)
{
	if (ThreadManager::Instance().m_WebSocketServerThread->isRunning()) {
		ui.listen_btn->setText(Close_TXT);
		ui.listen_btn->setCheckable(true);
		ui.listen_btn->setChecked(true);
		ui.port_box->setEnabled(false);
		ui.send_size->setEnabled(false);
	}
	else {
		ui.listen_btn->setText(Listen_TXT);
		ui.listen_btn->setChecked(false);
		ui.listen_btn->setCheckable(false);
		ui.port_box->setEnabled(true);
		ui.send_size->setEnabled(true);
	}
}

void WebSocketUI::onDataSize(const int len)
{
	m_DataSize += len;
	ui.send_size->setText(FormatBytes(m_DataSize));
}
