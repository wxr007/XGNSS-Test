#include "StreamCombine.h"
#include <QVBoxLayout>

StreamCombine::StreamCombine(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	serialPortWidget = new SerialportWidget(this);
	tcpClientWidget = new TcpClientWidget(this);
	ntripClient = new NtripClient(this);

	//// �������ֹ�����
	//QVBoxLayout* layout = new QVBoxLayout(this);
	//// ���ò��ֹ������ı߾�ͼ��
	//layout->setContentsMargins(0, 0, 0, 0);
	//layout->setSpacing(0);

	// ���Ӵ�����ӵ����ֹ�������
	ui.verticalLayout_1->addWidget(serialPortWidget);
	ui.verticalLayout_2->addWidget(tcpClientWidget);
	ui.verticalLayout_3->addWidget(ntripClient);
	//layout->addStretch(1);

	// ���������ڵĲ���Ϊ���ֹ�����
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
