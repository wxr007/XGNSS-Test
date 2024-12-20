#include "TcpClientWidget.h"
#include "ThreadManager.h"
#include "DataCache.h"
#include <QJsonObject>
#include <QJsonArray>

#define Connect_TXT u8"Connect"
#define Disconnect_TXT u8"Disconnect"

TcpClientWidget::TcpClientWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

    connect(ui.connect_btn, SIGNAL(clicked()), this, SLOT(onStartClicked()));
    connect(ThreadManager::Instance().m_TcpClientThread, SIGNAL(sgnEnable(bool)), this, SLOT(onEnable(bool)), Qt::QueuedConnection);
    connect(ThreadManager::Instance().m_SerialThread, SIGNAL(sgnEnable(bool)), this, SLOT(onOtherStreamEnable(bool)), Qt::QueuedConnection);
}

TcpClientWidget::~TcpClientWidget()
{}

void TcpClientWidget::loadConfig(QJsonObject& json)
{
    QJsonObject config = json["TcpClient"].toObject();
    if (config.isEmpty())
        return;
    //¼ÓÔØhost_boxÅäÖÃ
    QJsonObject host_json = config["host"].toObject();
    ui.host_box->loadJson(host_json);
    //¼ÓÔØport_boxÅäÖÃ
    QJsonObject port_json = config["port"].toObject();
    ui.port_box->loadJson(port_json);
    //...
}

void TcpClientWidget::saveConfig(QJsonObject& json)
{
    QJsonObject config = json["TcpClient"].toObject();
    //±£´æhost_boxÅäÖÃ
    QJsonObject host_json;
    ui.host_box->saveJson(host_json);
    config.insert("host", host_json);
    //±£´æport_boxÅäÖÃ
    QJsonObject port_json;
    ui.port_box->saveJson(port_json);
    config.insert("port", port_json);
    //...
    //±£´æÈ«²¿ÅäÖÃ
    json.insert("TcpClient", config);
}

void TcpClientWidget::onStartClicked()
{
    if (ThreadManager::Instance().m_TcpClientThread->isRunning())
    {
        ThreadManager::Instance().m_TcpClientThread->stop();
    }
    else {
        ui.host_box->addCurruntText(ui.host_box->currentText().trimmed());
        ui.port_box->addCurruntText(ui.port_box->currentText().trimmed());

        ThreadManager::Instance().m_TcpClientThread->setTcpClientParams(ui.host_box->currentText(), ui.port_box->currentText().toInt());
        ThreadManager::Instance().m_TcpClientThread->setDataFormat(Hemisphere_Format);
        ThreadManager::Instance().m_TcpClientThread->start();
    }
}

void TcpClientWidget::onEnable(bool enable)
{
    if (ThreadManager::Instance().m_TcpClientThread->isRunning()) {
        ui.connect_btn->setText(Disconnect_TXT);
        ui.connect_btn->setCheckable(true);
        ui.connect_btn->setChecked(true);
        ui.host_box->setEnabled(false);
        ui.port_box->setEnabled(false);
    }
    else {
        ui.connect_btn->setText(Connect_TXT);
        ui.connect_btn->setChecked(false);
        ui.connect_btn->setCheckable(false);
        ui.host_box->setEnabled(true);
        ui.port_box->setEnabled(true);
    }
}

void TcpClientWidget::onOtherStreamEnable(bool enable)
{
    if (enable) {
        ui.connect_btn->setEnabled(false);
    }
    else {
        ui.connect_btn->setEnabled(true);
    }
}

