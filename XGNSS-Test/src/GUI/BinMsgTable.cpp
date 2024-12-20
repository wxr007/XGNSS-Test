#include "BinMsgTable.h"
#include "ThreadManager.h"

BinMsgTable::BinMsgTable(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setTableBase(ui.msgTable);
	setTableBase(ui.bin3Table);
	setTableBase(ui.bin111Table);
	setTableBase(ui.bin112Table);
	initBIN3Rows();
	initBIN111Rows();
	initBIN112Rows();

	connect(ThreadManager::Instance().m_DataParser, SIGNAL(sgnUpdateHemisphereMsg(int)), this, SLOT(onUpdateHemisphereMsg(int)), Qt::QueuedConnection);
}

BinMsgTable::~BinMsgTable()
{}

void BinMsgTable::setTableBase(QTableWidget* table)
{
	table->setAlternatingRowColors(true);												//开启交替行背景色,在设置style为交替颜色时必须开启
	table->verticalHeader()->setVisible(false);											//隐藏垂直表头
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);				//表头自适应宽度
	table->setSelectionBehavior(QAbstractItemView::SelectRows);							//设置为选中行模式
	table->setFocusPolicy(Qt::NoFocus);													//去除选中虚线框
	table->setColumnCount(2);															//设置列数
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);							//设置只读

	QPalette palette = table->palette();
	palette.setColor(QPalette::Inactive, QPalette::Highlight, Qt::transparent);
	table->setPalette(palette);
}

void BinMsgTable::initBIN3Rows()
{
	QStringList row_titles = { u8"GPS周内秒" ,u8"GPS周",u8"卫星跟踪数",u8"卫星使用数",u8"定位模式",u8"纬度",u8"经度",u8"高程"
		,u8"水平速度",u8"垂直速度",u8"对地航向",u8"俯仰角",u8"翻滚角",u8"差分龄期",u8"姿态状态",u8"航向角标准差",u8"俯仰角标准差"
		,u8"HRMS",u8"VRMS",u8"HDOP",u8"VDOP",u8"TDOP",u8"CovNN",u8"CovNE",u8"CovNU",u8"CovEE",u8"CovEU",u8"CovUU" };

	for (const QString& str : row_titles) {
		int row = ui.bin3Table->rowCount(); // 获取当前行数
		ui.bin3Table->insertRow(row); // 插入新行
		QTableWidgetItem* item = new QTableWidgetItem;
		item->setText(str);
		ui.bin3Table->setItem(row, 0, item);
	}
}

void BinMsgTable::initBIN111Rows()
{
	QStringList row_titles = { u8"GPS周内秒" ,u8"GPS周",u8"加计x",u8"加计y",u8"加计z"
		,u8"陀螺x",u8"陀螺y",u8"陀螺z",u8"磁x",u8"磁y",u8"磁z",u8"温度" };

	for (const QString& str : row_titles) {
		int row = ui.bin111Table->rowCount(); // 获取当前行数
		ui.bin111Table->insertRow(row); // 插入新行
		QTableWidgetItem* item = new QTableWidgetItem;
		item->setText(str);
		ui.bin111Table->setItem(row, 0, item);
	}
}

void BinMsgTable::initBIN112Rows()
{
	QStringList row_titles = { u8"GPS周内秒" ,u8"GPS周",u8"导航模式",u8"状态"
		,u8"纬度",u8"经度",u8"高程"
		,u8"速度-北",u8"速度-东",u8"速度-下"
		,u8"横滚角",u8"俯仰角",u8"方位角"
		,u8"位置标准差-北",u8"位置标准差-东",u8"位置标准差-下"
		,u8"速度标准差-北",u8"速度标准差-东",u8"速度标准差-下"
		,u8"横滚角标准差",u8"俯仰角标准差",u8"方位角标准差"
		,u8"陀螺偏差x",u8"陀螺偏差y",u8"陀螺偏差z"
		,u8"加计偏差x",u8"加计偏差y",u8"加计偏差z" };

	for (const QString& str : row_titles) {
		int row = ui.bin112Table->rowCount(); // 获取当前行数
		ui.bin112Table->insertRow(row); // 插入新行
		QTableWidgetItem* item = new QTableWidgetItem;
		item->setText(str);
		ui.bin112Table->setItem(row, 0, item);
	}
}

void BinMsgTable::setItemString(QTableWidget* table, int row, int colum, QString str)
{
	QTableWidgetItem* item = table->item(row, colum);
	if (item) {
		item->setText(str);
	}
	else {
		item = new QTableWidgetItem;
		table->setItem(row, colum, item);
		item->setText(str);
	}
}

void BinMsgTable::setItemInt(QTableWidget* table, int row, int colum, const int value)
{
	setItemString(table, row, colum, QString::number(value));
}

void BinMsgTable::setItemValueString(QTableWidget* table, int row, QString str)
{
	setItemString(table, row, 1, str);
}

void BinMsgTable::setItemValue(QTableWidget* table, int row, double value, int prec)
{
	setItemValueString(table,row, QString::number(value, 'f', prec));
}

void BinMsgTable::setItemValue(QTableWidget* table, int row, int value)
{
	setItemValueString(table,row, QString::number(value));
}

void BinMsgTable::setMsg3TableData(SBinaryMsg3* sMsg003)
{
	setItemValue(ui.bin3Table, 0, sMsg003->m_dGPSTimeOfWeek,2);
	setItemValue(ui.bin3Table, 1, sMsg003->m_wGPSWeek);
	setItemValue(ui.bin3Table, 2, sMsg003->m_wNumSatsTracked);
	setItemValue(ui.bin3Table, 3, sMsg003->m_wNumSatsUsed);
	setItemValue(ui.bin3Table, 4, sMsg003->m_byNavMode);

	setItemValue(ui.bin3Table, 5, sMsg003->m_dLatitude, 10);
	setItemValue(ui.bin3Table, 6, sMsg003->m_dLongitude, 10);
	setItemValue(ui.bin3Table, 7, sMsg003->m_fHeight,2);

	setItemValue(ui.bin3Table, 8, sMsg003->m_fSpeed, 2);
	setItemValue(ui.bin3Table, 9, sMsg003->m_fVUp, 2);
	setItemValue(ui.bin3Table, 10, sMsg003->m_fCOG, 1);

	setItemValue(ui.bin3Table, 11, sMsg003->m_fPitch, 1);
	setItemValue(ui.bin3Table, 12, sMsg003->m_fRoll, 1);

	setItemValue(ui.bin3Table, 13, sMsg003->m_wAgeOfDiff, 1);
	setItemValue(ui.bin3Table, 14, sMsg003->m_wAttitudeStatus);

	setItemValue(ui.bin3Table, 15, sMsg003->m_fStdevHeading, 2);
	setItemValue(ui.bin3Table, 16, sMsg003->m_fStdevPitch, 2);

	setItemValue(ui.bin3Table, 17, sMsg003->m_fHRMS, 2);
	setItemValue(ui.bin3Table, 18, sMsg003->m_fVRMS, 2);

	setItemValue(ui.bin3Table, 19, sMsg003->m_fHDOP, 2);
	setItemValue(ui.bin3Table, 20, sMsg003->m_fVDOP, 2);
	setItemValue(ui.bin3Table, 21, sMsg003->m_fTDOP, 2);

	setItemValue(ui.bin3Table, 22, sMsg003->m_fCovNN, 2);
	setItemValue(ui.bin3Table, 23, sMsg003->m_fCovNE, 2);
	setItemValue(ui.bin3Table, 24, sMsg003->m_fCovNU, 2);
	setItemValue(ui.bin3Table, 25, sMsg003->m_fCovEE, 2);
	setItemValue(ui.bin3Table, 26, sMsg003->m_fCovEU, 2);
	setItemValue(ui.bin3Table, 27, sMsg003->m_fCovUU, 2);
}

void BinMsgTable::setMsg111TableData(SBinaryMsg111* sMsg111)
{
	setItemValue(ui.bin111Table, 0, sMsg111->m_dGPSTimeOfWeek, 2);
	setItemValue(ui.bin111Table, 1, sMsg111->m_wGPSWeek);

	setItemValue(ui.bin111Table, 2, sMsg111->m_fAccl_x, 2);
	setItemValue(ui.bin111Table, 3, sMsg111->m_fAccl_y, 2);
	setItemValue(ui.bin111Table, 4, sMsg111->m_fAccl_z, 2);

	setItemValue(ui.bin111Table, 5, sMsg111->m_fGyro_x, 2);
	setItemValue(ui.bin111Table, 6, sMsg111->m_fGyro_y, 2);
	setItemValue(ui.bin111Table, 7, sMsg111->m_fGyro_z, 2);

	setItemValue(ui.bin111Table, 8, sMsg111->m_fMag_x, 2);
	setItemValue(ui.bin111Table, 9, sMsg111->m_fMag_y, 2);
	setItemValue(ui.bin111Table, 10, sMsg111->m_fMag_z, 2);

	setItemValue(ui.bin111Table, 11, sMsg111->m_fTemperature, 2);
}

void BinMsgTable::setMsg112TableData(SBinaryMsg112* sMsg112)
{
	setItemValue(ui.bin112Table, 0, sMsg112->m_dGPSTimeOfWeek, 2);
	setItemValue(ui.bin112Table, 1, sMsg112->m_wGPSWeek);

	setItemValue(ui.bin112Table, 2, sMsg112->m_wNavMode);
	setItemValue(ui.bin112Table, 3, sMsg112->m_lStatus);

	setItemValue(ui.bin112Table, 4, sMsg112->m_dlatitude, 10);
	setItemValue(ui.bin112Table, 5, sMsg112->m_dlongitude, 10);
	setItemValue(ui.bin112Table, 6, sMsg112->m_fHeight, 2);

	setItemValue(ui.bin112Table, 7, sMsg112->m_fVelocityNorth, 2);
	setItemValue(ui.bin112Table, 8, sMsg112->m_fVelocityEast, 2);
	setItemValue(ui.bin112Table, 9, sMsg112->m_fVelocityDown, 2);

	setItemValue(ui.bin112Table, 10, sMsg112->m_fRoll, 2);
	setItemValue(ui.bin112Table, 11, sMsg112->m_fPitch, 2);
	setItemValue(ui.bin112Table, 12, sMsg112->m_fAzimuth, 2);

	setItemValue(ui.bin112Table, 13, sMsg112->m_fPosStdNorth, 2);
	setItemValue(ui.bin112Table, 14, sMsg112->m_fPosStdEast, 2);
	setItemValue(ui.bin112Table, 15, sMsg112->m_fPosStdDown, 2);

	setItemValue(ui.bin112Table, 16, sMsg112->m_fVelStdNorth, 2);
	setItemValue(ui.bin112Table, 17, sMsg112->m_fVelStdEast, 2);
	setItemValue(ui.bin112Table, 18, sMsg112->m_fVelStdDown, 2);

	setItemValue(ui.bin112Table, 19, sMsg112->m_fRollStd, 2);
	setItemValue(ui.bin112Table, 20, sMsg112->m_fPitchStd, 2);
	setItemValue(ui.bin112Table, 21, sMsg112->m_fAzimuthStd, 2);

	setItemValue(ui.bin112Table, 22, sMsg112->m_fGyroBias_x, 2);
	setItemValue(ui.bin112Table, 23, sMsg112->m_fGyroBias_y, 2);
	setItemValue(ui.bin112Table, 24, sMsg112->m_fGyroBias_z, 2);

	setItemValue(ui.bin112Table, 25, sMsg112->m_fAcclBias_x, 2);
	setItemValue(ui.bin112Table, 26, sMsg112->m_fAcclBias_y, 2);
	setItemValue(ui.bin112Table, 27, sMsg112->m_fAcclBias_z, 2);
}

void BinMsgTable::updataMsgCountTable()
{
	std::map<int, int> HemisphereMsg = DataCache::Instance().mHemisphereMsgCount;
	std::map<int, int>::iterator it = HemisphereMsg.begin();
	int i = 0;
	for (it = HemisphereMsg.begin(); it != HemisphereMsg.end(); it++) {
		int row = ui.msgTable->rowCount(); // 获取当前行数
		if (i >= row) {
			ui.msgTable->insertRow(row); // 插入新行
		}
		setItemInt(ui.msgTable, i, 0, it->first);
		setItemInt(ui.msgTable, i, 1, it->second);
		i++;
	}	
}

void BinMsgTable::onUpdateHemisphereMsg(int type)
{
	if (eMsg3 == type) {
		setMsg3TableData(&DataCache::Instance().sMsg003);
	}
	else if (eMsg112 == type) {
		setMsg112TableData(&DataCache::Instance().sMsg112);
	}
	else if (eMsg111 == type) {
		setMsg111TableData(&DataCache::Instance().sMsg111);
	}
	updataMsgCountTable();
}
