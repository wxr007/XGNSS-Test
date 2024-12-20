#include "StreamEasy.h"
#include "ThreadManager.h"
#include <QFontDatabase>

#define Start_TXT u8"Start"
#define Stop_TXT u8"Stop"

StreamEasy::StreamEasy(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::StreamEasyClass())
{
	ui->setupUi(this);
    //引入图形字体
    int fontId = QFontDatabase::addApplicationFont(":/XGNSSTest/res/fontawesome-webfont.ttf");
    QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont iconFont = QFont(fontName);
    ui->setting_btn->setFont(iconFont);
    ui->setting_btn->setText(QChar(0xf013));

	connect(ui->start_btn, SIGNAL(clicked()), this, SLOT(onStartClicked()));
    connect(ui->setting_btn, SIGNAL(toggled(bool)), this, SIGNAL(sgnSettingToggled(bool)));
	connect(ThreadManager::Instance().m_TcpClientThread, SIGNAL(sgnEnable(bool)), this, SLOT(onEnable(bool)), Qt::QueuedConnection);
	connect(ThreadManager::Instance().m_SerialThread, SIGNAL(sgnEnable(bool)), this, SLOT(onEnable(bool)), Qt::QueuedConnection);
}

StreamEasy::~StreamEasy()
{
	delete ui;
}

void StreamEasy::setSettingAction(QAction* action)
{
    ui->setting_btn->setDefaultAction(action);
}

void StreamEasy::onStartClicked()
{
    if (ui->streamType->currentIndex() == 0) //Serial
    {
        emit sgnStartSerialport();
    }
    else if (ui->streamType->currentIndex() == 1) //Tcp
    {
        emit sgnStartTcpClient();
    }
    if (ui->Ntrip->isChecked()) {
        emit sgnStartNtrip();
    }
}

void StreamEasy::onEnable(bool enable)
{
    if (ThreadManager::Instance().m_TcpClientThread->isRunning() || ThreadManager::Instance().m_SerialThread->isRunning()) {
        ui->start_btn->setText(Stop_TXT);
        ui->start_btn->setCheckable(true);
        ui->start_btn->setChecked(true);
        ui->streamType->setEnabled(false);
        ui->Ntrip->setEnabled(false);
    }
    else {
        ui->start_btn->setText(Start_TXT);
        ui->start_btn->setChecked(false);
        ui->start_btn->setCheckable(false);
        ui->streamType->setEnabled(true);
        ui->Ntrip->setEnabled(true);
    }
}
