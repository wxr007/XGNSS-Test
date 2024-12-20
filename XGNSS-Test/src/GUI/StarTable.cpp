#include "StarTable.h"
#include "ThreadManager.h"

bool compare_satid(const UnfoldMsg309Sat& first, const UnfoldMsg309Sat& second)
{
	return first.satId < second.satId;
}

#define Default_SNR 30
#define GPS_COL_NUM 10
#define GAL_COL_NUM 10
#define GLO_COL_NUM 10
#define BDS_COL_NUM 13
#define QZS_COL_NUM 10
#define IRN_COL_NUM 6

StarTable::StarTable(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	mGPSM = new QStandardItemModel(this);
	mGPSM->setColumnCount(GPS_COL_NUM);
	mGPSM->setHorizontalHeaderLabels(QStringList({ "GPS","L1CA","L2P","L2C","L5","L1C",u8"方位角",u8"高度角",u8"在使用",u8"健康" }));
	ui.GPS_V->setModel(mGPSM);
	setTableView(ui.GPS_V);

	mGALM = new QStandardItemModel(this);
	mGALM->setColumnCount(GAL_COL_NUM);
	mGALM->setHorizontalHeaderLabels(QStringList({ "GAL","E1","E5A","E5B","E6","ALTBOC",u8"方位角",u8"高度角",u8"在使用",u8"健康" }));
	ui.GAL_V->setModel(mGALM);
	setTableView(ui.GAL_V);

	mGLOM = new QStandardItemModel(this);
	mGLOM->setColumnCount(GLO_COL_NUM);
	mGLOM->setHorizontalHeaderLabels(QStringList({ "GLO","G1","G2","G1OC","G2OC","G3",u8"方位角",u8"高度角",u8"在使用",u8"健康" }));
	ui.GLO_V->setModel(mGLOM);
	setTableView(ui.GLO_V);

	mBDSM = new QStandardItemModel(this);
	mBDSM->setColumnCount(BDS_COL_NUM);
	mBDSM->setHorizontalHeaderLabels(QStringList({ "BDS","B1","B2","B3","B1C","B2A","B2B","B3C","ACEBOC",u8"方位角",u8"高度角",u8"在使用",u8"健康"}));
	ui.BDS_V->setModel(mBDSM);
	setTableView(ui.BDS_V);

	mQZSM = new QStandardItemModel(this);
	mQZSM->setColumnCount(QZS_COL_NUM);
	mQZSM->setHorizontalHeaderLabels(QStringList({ "QZS","L1CA","L2C","L5","L1C","LEX",u8"方位角",u8"高度角",u8"在使用",u8"健康" }));
	ui.QZS_V->setModel(mQZSM);
	setTableView(ui.QZS_V);

	mIRNM = new QStandardItemModel(this);
	mIRNM->setColumnCount(IRN_COL_NUM);
	mIRNM->setHorizontalHeaderLabels(QStringList({ "IRN","L5",u8"方位角",u8"高度角",u8"在使用",u8"健康" }));
	ui.IRN_V->setModel(mIRNM);
	setTableView(ui.IRN_V);

	unfoldMsg309.msg309Sats.clear();
	unfoldMsg309.m_Antenna = 0;
	unfoldMsg309.m_wGPSWeek = 0;
	unfoldMsg309.m_dGPSTimeOfWeek = 0;

	connect(ui.primary, SIGNAL(toggled(bool)), this, SLOT(onAntennaChanged()));
	connect(ThreadManager::Instance().m_DataParser, SIGNAL(sgnUpdateHemisphereMsg(int)), this, SLOT(onUpdateHemisphereMsg(int)), Qt::QueuedConnection);
	connect(this, SIGNAL(sgnSetAntenna(int)), ThreadManager::Instance().m_DataParser, SLOT(onSetAntenna(int)), Qt::QueuedConnection);

	ui.splitter_2->setStretchFactor(0, 13);
	ui.splitter_2->setStretchFactor(1, 10);
	ui.splitter_2->setStretchFactor(2, 6);
}

StarTable::~StarTable()
{
}

void StarTable::setTableView(QTableView* table)
{
	table->setAlternatingRowColors(true);												//开启交替行背景色,在设置style为交替颜色时必须开启
	table->verticalHeader()->setVisible(false);											//隐藏水平表头
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);				//设置列自适应宽度
	//table->horizontalHeader()->setStretchLastSection(true);							//设置最后一栏自适应长度
	table->setSelectionBehavior(QAbstractItemView::SelectRows);							//设置为选中行模式
	table->setFocusPolicy(Qt::NoFocus);													//去除选中虚线框
	//table->setColumnCount(2);															//设置列数
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);							//设置只读

	QPalette palette = table->palette();
	palette.setColor(QPalette::Inactive, QPalette::Highlight, Qt::transparent);
	table->setPalette(palette);
}

void StarTable::setModelItem(QStandardItemModel* model, int row, int colum, QString str)
{
	QModelIndex index = model->index(row, colum);
	model->setData(index, str);
}

void StarTable::setModelValue(QStandardItemModel* model, int row, int colum, double value, int prec)
{
	setModelItem(model, row, colum, QString::number(value, 'f', prec));
}

void StarTable::setModelValue(QStandardItemModel* model, int row, int colum, int value)
{
	setModelItem(model, row, colum, QString::number(value));
}

void StarTable::setTableUpdatesEnabled(bool enable)
{
	ui.GPS_V->setUpdatesEnabled(enable);
	ui.GLO_V->setUpdatesEnabled(enable);
	ui.GAL_V->setUpdatesEnabled(enable);
	ui.BDS_V->setUpdatesEnabled(enable);
	ui.QZS_V->setUpdatesEnabled(enable);
	ui.IRN_V->setUpdatesEnabled(enable);
}

void StarTable::setGPSData(std::vector<UnfoldMsg309Sat>& gps_stas)
{
	if (gps_stas.size() == 0)
		return;

	if (mGPSM->rowCount() != gps_stas.size() + 1) {
		mGPSM->setRowCount((int)gps_stas.size() + 1);
		for (int i = 0; i < GPS_COL_NUM; i++)
		{
			mGPSM->clearItemData(mGPSM->index((int)gps_stas.size(), i));
		}		
	}

	int used = 0;
	for (int i = 0; i < gps_stas.size(); i++) {
		UnfoldMsg309Sat& sat = gps_stas[i];
		setModelValue(mGPSM, i, 0, sat.satId);
		if (sat.signal[0]) setModelValue(mGPSM, i, 1, sat.SNR[0] + Default_SNR); else setModelItem(mGPSM, i, 1, "/");
		if (sat.signal[1]) setModelValue(mGPSM, i, 2, sat.SNR[1] + 18); else setModelItem(mGPSM, i, 2, "/");
		if (sat.signal[2]) setModelValue(mGPSM, i, 3, sat.SNR[2] + 18); else setModelItem(mGPSM, i, 3, "/");
		if (sat.signal[3]) setModelValue(mGPSM, i, 4, sat.SNR[3] + Default_SNR); else setModelItem(mGPSM, i, 4, "/");
		if (sat.signal[4]) setModelValue(mGPSM, i, 5, sat.SNR[4] + Default_SNR); else setModelItem(mGPSM, i, 5, "/");
		setModelValue(mGPSM, i, 6, sat.azimuth);
		setModelValue(mGPSM, i, 7, sat.elev);
		setModelValue(mGPSM, i, 8, sat.used);
		setModelValue(mGPSM, i, 9, sat.health);
		if (sat.used) {
			used++;
		}
	}
	setModelItem(mGPSM, (int)gps_stas.size(), 0, QString::asprintf("%d/%d", used, gps_stas.size()));
}
void StarTable::setGLOData(std::vector<UnfoldMsg309Sat>& gol_stas)
{
	if (gol_stas.size() == 0)
		return;

	if (mGLOM->rowCount() != gol_stas.size() + 1) {
		mGLOM->setRowCount((int)gol_stas.size() + 1);
		for (int i = 0; i < GLO_COL_NUM; i++)
		{
			mGLOM->clearItemData(mGLOM->index((int)gol_stas.size(), i));
		}
	}

	int used = 0;
	for (int i = 0; i < gol_stas.size(); i++) {
		UnfoldMsg309Sat& sat = gol_stas[i];
		setModelValue(mGLOM, i, 0, sat.satId);
		if (sat.signal[0]) setModelValue(mGLOM, i, 1, sat.SNR[0] + Default_SNR); else setModelItem(mGLOM, i, 1, "/");
		if (sat.signal[1]) setModelValue(mGLOM, i, 2, sat.SNR[1] + Default_SNR); else setModelItem(mGLOM, i, 2, "/");
		if (sat.signal[2]) setModelValue(mGLOM, i, 3, sat.SNR[2] + Default_SNR); else setModelItem(mGLOM, i, 3, "/");
		if (sat.signal[3]) setModelValue(mGLOM, i, 4, sat.SNR[3] + Default_SNR); else setModelItem(mGLOM, i, 4, "/");
		if (sat.signal[4]) setModelValue(mGLOM, i, 5, sat.SNR[4] + Default_SNR); else setModelItem(mGLOM, i, 5, "/");
		setModelValue(mGLOM, i, 6, sat.azimuth);
		setModelValue(mGLOM, i, 7, sat.elev);
		setModelValue(mGLOM, i, 8, sat.used);
		setModelValue(mGLOM, i, 9, sat.health);
		if (sat.used) {
			used++;
		}
	}
	setModelItem(mGLOM, (int)gol_stas.size(), 0, QString::asprintf("%d/%d", used, gol_stas.size()));
}
void StarTable::setGALData(std::vector<UnfoldMsg309Sat>& gal_stas)
{
	if (gal_stas.size() == 0)
		return;

	if (mGALM->rowCount() != gal_stas.size() + 1){
		mGALM->setRowCount((int)gal_stas.size() + 1);
		for (int i = 0; i < GAL_COL_NUM; i++)
		{
			mGALM->clearItemData(mGALM->index((int)gal_stas.size(), i));
		}
	}

	int used = 0;
	for (int i = 0; i < gal_stas.size(); i++) {
		UnfoldMsg309Sat& sat = gal_stas[i];
		setModelValue(mGALM, i, 0, sat.satId);
		if (sat.signal[0]) setModelValue(mGALM, i, 1, sat.SNR[0] + Default_SNR); else setModelItem(mGALM, i, 1, "/");
		if (sat.signal[1]) setModelValue(mGALM, i, 2, sat.SNR[1] + Default_SNR); else setModelItem(mGALM, i, 2, "/");
		if (sat.signal[2]) setModelValue(mGALM, i, 3, sat.SNR[2] + Default_SNR); else setModelItem(mGALM, i, 3, "/");
		if (sat.signal[3]) setModelValue(mGALM, i, 4, sat.SNR[3] + Default_SNR); else setModelItem(mGALM, i, 4, "/");
		if (sat.signal[4]) setModelValue(mGALM, i, 5, sat.SNR[4] + Default_SNR); else setModelItem(mGALM, i, 5, "/");
		setModelValue(mGALM, i, 6, sat.azimuth);
		setModelValue(mGALM, i, 7, sat.elev);
		setModelValue(mGALM, i, 8, sat.used);
		setModelValue(mGALM, i, 9, sat.health);
		if (sat.used) {
			used++;
		}
	}
	setModelItem(mGALM, (int)gal_stas.size(), 0, QString::asprintf("%d/%d", used, gal_stas.size()));
}
void StarTable::setBDSData(std::vector<UnfoldMsg309Sat>& bds_stas)
{
	if (bds_stas.size() == 0)
		return;

	if (mBDSM->rowCount() != bds_stas.size() + 1){
		mBDSM->setRowCount((int)bds_stas.size() + 1);
		for (int i = 0; i < BDS_COL_NUM; i++)
		{
			mBDSM->clearItemData(mBDSM->index((int)bds_stas.size(), i));
		}
	}

	int used = 0;
	for (int i = 0; i < bds_stas.size(); i++) {
		UnfoldMsg309Sat& sat = bds_stas[i];
		setModelValue(mBDSM, i, 0, sat.satId);
		if (sat.signal[0]) setModelValue(mBDSM, i, 1, sat.SNR[0] + Default_SNR); else setModelItem(mBDSM, i, 1, "/");
		if (sat.signal[1]) setModelValue(mBDSM, i, 2, sat.SNR[1] + Default_SNR); else setModelItem(mBDSM, i, 2, "/");
		if (sat.signal[2]) setModelValue(mBDSM, i, 3, sat.SNR[2] + Default_SNR); else setModelItem(mBDSM, i, 3, "/");
		if (sat.signal[3]) setModelValue(mBDSM, i, 4, sat.SNR[3] + Default_SNR); else setModelItem(mBDSM, i, 4, "/");
		if (sat.signal[4]) setModelValue(mBDSM, i, 5, sat.SNR[4] + Default_SNR); else setModelItem(mBDSM, i, 5, "/");
		if (sat.signal[5]) setModelValue(mBDSM, i, 6, sat.SNR[5] + Default_SNR); else setModelItem(mBDSM, i, 6, "/");
		if (sat.signal[6]) setModelValue(mBDSM, i, 7, sat.SNR[6] + Default_SNR); else setModelItem(mBDSM, i, 7, "/");
		if (sat.signal[7]) setModelValue(mBDSM, i, 8, sat.SNR[7] + Default_SNR); else setModelItem(mBDSM, i, 8, "/");
		setModelValue(mBDSM, i, 9, sat.azimuth);
		setModelValue(mBDSM, i, 10, sat.elev);
		setModelValue(mBDSM, i, 11, sat.used);
		setModelValue(mBDSM, i, 12, sat.health);
		if (sat.used) {
			used++;
		}
	}
	setModelItem(mBDSM, (int)bds_stas.size(), 0, QString::asprintf("%d/%d", used, bds_stas.size()));
}
void StarTable::setQZSData(std::vector<UnfoldMsg309Sat>& qzs_stas)
{
	if (qzs_stas.size() == 0)
		return;

	if (mQZSM->rowCount() != qzs_stas.size() + 1){
		mQZSM->setRowCount((int)qzs_stas.size() + 1);
		for (int i = 0; i < QZS_COL_NUM; i++)
		{
			mQZSM->clearItemData(mQZSM->index((int)qzs_stas.size(), i));
		}
	}

	int used = 0;
	for (int i = 0; i < qzs_stas.size(); i++) {
		UnfoldMsg309Sat& sat = qzs_stas[i];
		setModelValue(mQZSM, i, 0, sat.satId);
		if (sat.signal[0]) setModelValue(mQZSM, i, 1, sat.SNR[0] + Default_SNR);	else setModelItem(mQZSM, i, 1, "/");
		if (sat.signal[1]) setModelValue(mQZSM, i, 2, sat.SNR[1] + Default_SNR);	else setModelItem(mQZSM, i, 2, "/");
		if (sat.signal[2]) setModelValue(mQZSM, i, 3, sat.SNR[2] + Default_SNR);	else setModelItem(mQZSM, i, 3, "/");
		if (sat.signal[3]) setModelValue(mQZSM, i, 4, sat.SNR[3] + Default_SNR);	else setModelItem(mQZSM, i, 4, "/");
		if (sat.signal[4]) setModelValue(mQZSM, i, 5, sat.SNR[4] + Default_SNR);	else setModelItem(mQZSM, i, 5, "/");
		setModelValue(mQZSM, i, 6, sat.azimuth);
		setModelValue(mQZSM, i, 7, sat.elev);
		setModelValue(mQZSM, i, 8, sat.used);
		setModelValue(mQZSM, i, 9, sat.health);
		if (sat.used) {
			used++;
		}
	}

	setModelItem(mQZSM, (int)qzs_stas.size(), 0, QString::asprintf("%d/%d", used, qzs_stas.size()));
}
void StarTable::setIRNData(std::vector<UnfoldMsg309Sat>& irn_stas)
{
	if (irn_stas.size() == 0)
		return;

	if (mIRNM->rowCount() != irn_stas.size() + 1){
		mIRNM->setRowCount((int)irn_stas.size() + 1);
		for (int i = 0; i < IRN_COL_NUM; i++)
		{
			mIRNM->clearItemData(mIRNM->index((int)irn_stas.size(), i));
		}
	}

	int used = 0;
	for (int i = 0; i < irn_stas.size(); i++) {
		UnfoldMsg309Sat& sat = irn_stas[i];
		setModelValue(mIRNM, i, 0, sat.satId);
		if (sat.signal[0]) setModelValue(mIRNM, i, 1, sat.SNR[0] + Default_SNR);	else setModelItem(mIRNM, i, 1, "/");
		setModelValue(mIRNM, i, 2, sat.azimuth);
		setModelValue(mIRNM, i, 3, sat.elev);
		setModelValue(mIRNM, i, 4, sat.used);
		setModelValue(mIRNM, i, 5, sat.health);
		if (sat.used) {
			used++;
		}
	}
	setModelItem(mIRNM, (int)irn_stas.size(), 0, QString::asprintf("%d/%d", used, irn_stas.size()));
}
void StarTable::onAntennaChanged()
{
	if (ui.primary->isChecked()) {
		mGPSM->setRowCount(0);
		mGALM->setRowCount(0);
		mGLOM->setRowCount(0);
		mBDSM->setRowCount(0);
		mQZSM->setRowCount(0);
		mIRNM->setRowCount(0);
		emit sgnSetAntenna(0);
	}
	else if (ui.secondary->isChecked()) {
		mGPSM->setRowCount(0);
		mGALM->setRowCount(0);
		mGLOM->setRowCount(0);
		mBDSM->setRowCount(0);
		mQZSM->setRowCount(0);
		mIRNM->setRowCount(0);
		emit sgnSetAntenna(1);
	}
}
void StarTable::onUpdateHemisphereMsg(int type) 
{
	DataCache::Instance().m_mtx.lock();
	unfoldMsg309.msg309Sats.swap(DataCache::Instance().unfoldMsg309.msg309Sats);
	unfoldMsg309.m_Antenna = DataCache::Instance().unfoldMsg309.m_Antenna;
	unfoldMsg309.m_wGPSWeek = DataCache::Instance().unfoldMsg309.m_wGPSWeek;
	unfoldMsg309.m_dGPSTimeOfWeek = DataCache::Instance().unfoldMsg309.m_dGPSTimeOfWeek;
	DataCache::Instance().m_mtx.unlock();
	if (unfoldMsg309.msg309Sats.size() == 0) {
		return;
	}
	//unfoldMsg309.msg309Sats.sort(compare_satid);
	std::vector<UnfoldMsg309Sat> gps_stas;
	std::vector<UnfoldMsg309Sat> gol_stas;
	std::vector<UnfoldMsg309Sat> gal_stas;
	std::vector<UnfoldMsg309Sat> bds_stas;
	std::vector<UnfoldMsg309Sat> qzs_stas;
	std::vector<UnfoldMsg309Sat> irn_stas;
	std::list<UnfoldMsg309Sat>::iterator it;
	for (it = unfoldMsg309.msg309Sats.begin(); it != unfoldMsg309.msg309Sats.end(); it++) {
		if (it->satId == 0) {
			continue;
		}
		bool hasSignal = false;
		for (int i = 0; i < 8; i++) {
			if (it->signal[i] == 1) {
				hasSignal = true;
				break;
			}
		}
		if (!hasSignal) {
			continue;
		}
	    if (it->sys == HemiSys_GPS) {
	        gps_stas.push_back(*it);
	    }
	    else if (it->sys == HemiSys_GLONASS) {
	        gol_stas.push_back(*it);
	    }
	    else if (it->sys == HemiSys_GALILEO) {
	        gal_stas.push_back(*it);
	    }
	    else if (it->sys == HemiSys_BEIDOU) {
	        bds_stas.push_back(*it);
	    }
	    else if (it->sys == HemiSys_QZSS) {
	        qzs_stas.push_back(*it);
	    }
	    else if (it->sys == HemiSys_IRNSS) {
	        irn_stas.push_back(*it);
	    }
	}
	setTableUpdatesEnabled(false);
	setGPSData(gps_stas);
	setGALData(gal_stas);
	setGLOData(gol_stas);
	setBDSData(bds_stas);
	setQZSData(qzs_stas);
	setIRNData(irn_stas);
	setTableUpdatesEnabled(true);
}