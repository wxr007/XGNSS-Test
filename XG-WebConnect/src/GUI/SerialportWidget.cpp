#include "SerialportWidget.h"
#include <QFontDataBase>
#include <QMessageBox>
#include "ThreadManager.h"
#include "DataCache.h"
#include "helper/Utils.h"

#define Connect_TXT u8"Connect"
#define Disconnect_TXT u8"Disconnect"

SerialportWidget::SerialportWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    //引入图形字体
    int fontId = QFontDatabase::addApplicationFont(":/XGWebConnect/res/fontawesome-webfont.ttf");
    QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont iconFont = QFont(fontName);
    ui.refresh_btn->setFont(iconFont);
    ui.refresh_btn->setText(QChar(0xf021));
    ui.refresh_btn->setToolTip(u8"Refresh");
    searchPorts();
    connect(ui.refresh_btn, SIGNAL(clicked()), this, SLOT(onRefreshClicked()));
    connect(ui.start_btn, SIGNAL(clicked()), this, SLOT(onStartClicked()));

    connect(ThreadManager::Instance().m_SerialThread, SIGNAL(sgnEnable(bool)), this, SLOT(onEnable(bool)), Qt::QueuedConnection);
    connect(ThreadManager::Instance().m_SerialThread, SIGNAL(sgnOpenFailed()), this, SLOT(onOpenFailed()), Qt::QueuedConnection);
    connect(ThreadManager::Instance().m_SerialThread, SIGNAL(sgnDataSize(const int)), this, SLOT(onDataSize(const int)), Qt::QueuedConnection);
    connect(ThreadManager::Instance().m_TcpClientThread, SIGNAL(sgnEnable(bool)), this, SLOT(onOtherStreamEnable(bool)), Qt::QueuedConnection);
}

SerialportWidget::~SerialportWidget()
{}

void SerialportWidget::loadConfig(QJsonObject& json)
{
    QJsonObject config = json["serial"].toObject();
    if (config.isEmpty())
        return;
    ui.port_box->setCurrentIndex(config["port"].toInt());
    ui.baud_box->setCurrentText(config["baud"].toString());
}

void SerialportWidget::saveConfig(QJsonObject& json)
{
    QJsonObject config;
    config.insert("port", ui.port_box->currentIndex());
    config.insert("baud", ui.baud_box->currentText());
    json.insert("serial", config);
}

void SerialportWidget::searchPorts()
{
    ui.port_box->clear();
    //通过QSerialPortInfo查找可用串口
    foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts())
    {
        QString showName = info.portName();
        ui.port_box->addItem(showName);
    }
}
void SerialportWidget::onRefreshClicked()
{
    searchPorts();
}

void SerialportWidget::onStartClicked()
{
    if (ThreadManager::Instance().m_SerialThread->isRunning()){
        ThreadManager::Instance().m_SerialThread->stop();
    }
    else {
        ThreadManager::Instance().m_SerialThread->setSerialPortParams(ui.port_box->currentText(), ui.baud_box->currentText().toInt());
        ThreadManager::Instance().m_SerialThread->setDataFormat(Hemisphere_Format);
        ThreadManager::Instance().m_SerialThread->setLogFile(false);
        ThreadManager::Instance().m_SerialThread->start();  
    }
}

void SerialportWidget::onEnable(bool enable)
{
    if (ThreadManager::Instance().m_SerialThread->isRunning()) {
        ui.start_btn->setText(Disconnect_TXT);
        ui.start_btn->setCheckable(true);
        ui.start_btn->setChecked(true);
        ui.refresh_btn->setEnabled(false);
        ui.port_box->setEnabled(false);
        ui.baud_box->setEnabled(false);
    }
    else {
        ui.start_btn->setText(Connect_TXT);
        ui.start_btn->setChecked(false);
        ui.start_btn->setCheckable(false);
        ui.refresh_btn->setEnabled(true);
        ui.port_box->setEnabled(true);
        ui.baud_box->setEnabled(true);
    }
}

void SerialportWidget::onOpenFailed()
{
    ThreadManager::Instance().m_SerialThread->stop();
    QMessageBox::warning(this, u8"警告", u8"串口打开失败!");
}

void SerialportWidget::onOtherStreamEnable(bool enable)
{
    if (enable) {
        ui.start_btn->setEnabled(false);
    }
    else {
        ui.start_btn->setEnabled(true);
    }
}

void SerialportWidget::onDataSize(const int len)
{
    m_DataSize += len;
    ui.send_size->setText(FormatBytes(m_DataSize));
}

