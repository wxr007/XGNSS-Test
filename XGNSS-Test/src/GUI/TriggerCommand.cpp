#include "TriggerCommand.h"
#include "ThreadManager.h"
#include "DataCache.h"

TriggerCommand::TriggerCommand(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initValues();

	connect(ui.trigger, SIGNAL(toggled(bool)), this, SLOT(onChangeTrigger(bool)));
	connect(ThreadManager::Instance().m_DataParser, SIGNAL(sgnUpdateNMEA(int)), this, SLOT(onUpdateNMEA(int)), Qt::QueuedConnection);
}

TriggerCommand::~TriggerCommand()
{}

void TriggerCommand::initValues()
{
	mStatisticCount = 0;
}

void TriggerCommand::totalPostType(int posType)
{
	if (ui.trigger->isChecked()) {
		if (ui.statisticType->currentIndex() == 0 && posType == 1) {
			mStatisticCount++;
		}
		else if (ui.statisticType->currentIndex() == 1 && posType == 2) {
			mStatisticCount++;
		}
		else if (ui.statisticType->currentIndex() == 2 && posType == 4) {
			mStatisticCount++;
		}
		else if (ui.statisticType->currentIndex() == 3 && posType == 5) {
			mStatisticCount++;
		}		
		if (mStatisticCount >= ui.statisticCount->value()) {
			QString content = ui.send_txt->toPlainText();
			if (!content.trimmed().isEmpty()) {
				emit sgnAutoCommand(content);
			}			
			ui.trigger->setChecked(false);
		}
	}
}

void TriggerCommand::onUpdateNMEA(int type)
{
	if ($GNGGA == type) {
		nmea_gga_t& m_nmea_gga = DataCache::Instance().m_nmea_gga;
		totalPostType(m_nmea_gga.type);
	}
}

void TriggerCommand::onChangeTrigger(bool checked) {
	initValues();
	if (checked) {
		ui.container->setEnabled(false);
	}
	else {
		ui.container->setEnabled(true);
	}
}