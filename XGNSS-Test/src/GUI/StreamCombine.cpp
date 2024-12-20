#include "StreamCombine.h"
#include <QVBoxLayout>

StreamCombine::StreamCombine(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	serialPortWidget = new SerialportWidget(this);
	tcpClientWidget = new TcpClientWidget(this);
	ntripClient = new NtripClient(this);

	//// 创建布局管理器
	//QVBoxLayout* layout = new QVBoxLayout(this);
	//// 设置布局管理器的边距和间距
	//layout->setContentsMargins(0, 0, 0, 0);
	//layout->setSpacing(0);

	// 将子窗口添加到布局管理器中
	ui.verticalLayout_1->addWidget(serialPortWidget);
	ui.verticalLayout_2->addWidget(tcpClientWidget);
	ui.verticalLayout_3->addWidget(ntripClient);
	//layout->addStretch(1);

	// 设置主窗口的布局为布局管理器
	//this->setLayout(layout);
}

StreamCombine::~StreamCombine()
{}

void StreamCombine::loadConfig(QJsonObject & json)
{
	serialPortWidget->loadConfig(json);
	tcpClientWidget->loadConfig(json);
	ntripClient->loadConfig(json);
}

void StreamCombine::saveConfig(QJsonObject& json)
{
	serialPortWidget->saveConfig(json);
	tcpClientWidget->saveConfig(json);
	ntripClient->saveConfig(json);
}
