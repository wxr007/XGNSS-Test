#include "XGWebConnect.h"

XGWebConnect::XGWebConnect(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    m_SerialportWidget = new SerialportWidget(NULL);
    ui.tabWidget->addTab(m_SerialportWidget, u8"Serialport");

    m_NtripClient = new NtripClient(NULL);
    ui.tabWidget->addTab(m_NtripClient, u8"Ntrip");

    m_WebSocketUI = new WebSocketUI(NULL);
    ui.tabWidget->addTab(m_WebSocketUI, u8"WebSocket");
}

XGWebConnect::~XGWebConnect()
{}
