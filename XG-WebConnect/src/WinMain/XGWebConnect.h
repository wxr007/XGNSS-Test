#pragma once

#include <QtWidgets/QWidget>
#include "ui_XGWebConnect.h"
#include "SerialportWidget.h"
#include "NtripClient.h"
#include "WebSocketUI.h"

class XGWebConnect : public QWidget
{
    Q_OBJECT

public:
    XGWebConnect(QWidget *parent = nullptr);
    ~XGWebConnect();

private:
    Ui::XGWebConnectClass ui;
    SerialportWidget* m_SerialportWidget;
    NtripClient* m_NtripClient;
    WebSocketUI* m_WebSocketUI;
};
