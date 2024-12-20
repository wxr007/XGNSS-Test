#include "DataTableUI.h"
#include <QTableView>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QColor>
#include "ThreadManager.h"
#include "DataCache.h"
#include "rtkcmn.h"
#include "helper/Utils.h"
#include "gnss_math.h"

DataTableUI::DataTableUI(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setTableBase(ui.tableWidget);
	setTableBase(ui.tableWidget_2);
	initTable1Rows();
	initTable2Rows();
	initValues();
	mPosTypeColor << QColor(245, 108, 108) << QColor(182, 114, 178) << QColor(255, 255, 255) << QColor(103, 194, 58) << QColor(230, 162, 60);

	connect(ui.ClearDataBtn,SIGNAL(clicked()),this,SLOT(onClearData()));
	connect(ui.ClearCountBtn,SIGNAL(clicked()),this,SLOT(onClearCount()));
	connect(ThreadManager::Instance().m_SerialThread, SIGNAL(sgnEnable(bool)), this, SLOT(onOpen(bool)), Qt::QueuedConnection);
	connect(ThreadManager::Instance().m_TcpClientThread, SIGNAL(sgnEnable(bool)), this, SLOT(onOpen(bool)), Qt::QueuedConnection);
	connect(ThreadManager::Instance().m_DataParser, SIGNAL(sgnUpdateNMEA(int)), this, SLOT(onUpdateNMEA(int)), Qt::QueuedConnection);
	connect(ThreadManager::Instance().m_SerialThread, SIGNAL(sgnDataSize(const int)), this, SLOT(onDataSize(const int)), Qt::QueuedConnection);
	connect(ThreadManager::Instance().m_TcpClientThread, SIGNAL(sgnDataSize(const int)), this, SLOT(onDataSize(const int)), Qt::QueuedConnection);

	mStreamEasy = new StreamEasy(this);
	ui.verticalLayout->addWidget(mStreamEasy);
}

DataTableUI::~DataTableUI()
{}

void DataTableUI::setTableBase(QTableWidget* table)
{
	table->setAlternatingRowColors(true);													//���������б���ɫ,������styleΪ������ɫʱ���뿪��
	table->horizontalHeader()->setVisible(false);											//����ˮƽ��ͷ
	table->verticalHeader()->setVisible(false);												//���ش�ֱ��ͷ
	//table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);				//��ͷ����Ӧ���
	table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);					//����Ӧ�߶�
	//table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);		//��������������Ӧ���
	table->horizontalHeader()->setStretchLastSection(true);									//�������һ������Ӧ����
	table->setSelectionMode(QAbstractItemView::NoSelection);								//����Ϊ��ѡ��ģʽ
	//table->setSelectionBehavior(QAbstractItemView::SelectRows);							//����Ϊѡ����ģʽ
	table->setFocusPolicy(Qt::NoFocus);														//ȥ��ѡ�����߿�
	//table->setContextMenuPolicy(Qt::CustomContextMenu);									//��Ҫ���Ҽ��˵�����,���������
	table->setColumnCount(2);																//��������
	//table->setRowCount(5);																//��������
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);								//����ֻ��
	//ui.tableWidget->setEnabled(false);													//���ñ��
	table->setColumnWidth(0, 75);

	QPalette palette = ui.tableWidget->palette();
	palette.setColor(QPalette::Inactive, QPalette::Highlight, Qt::transparent);
	ui.tableWidget->setPalette(palette);
}

void DataTableUI::initTable1Rows()
{
	QStringList row_titles = { u8"�̶�����" ,u8"�������",u8"��ֽ���",u8"�������",u8"UTCʱ��",u8"�ӳ�",u8"ƽ���ӳ�",u8"��վID",u8"RxNum"};

	for (const QString& str : row_titles) {
		int row = ui.tableWidget->rowCount(); // ��ȡ��ǰ����
		ui.tableWidget->insertRow(row); // ��������
		QTableWidgetItem* item = new QTableWidgetItem;
		item->setText(str);
		ui.tableWidget->setItem(row, 0, item);
	}
}

void DataTableUI::initTable2Rows()
{
	QStringList row_titles = {u8"��״̬",u8"����x",u8"����y",u8"����h",u8"HRMS",u8"VRMS",u8"Roll",u8"Pitch",u8"Yaw" };

	for (const QString& str : row_titles) {
		int row = ui.tableWidget_2->rowCount(); // ��ȡ��ǰ����
		ui.tableWidget_2->insertRow(row); // ��������
		QTableWidgetItem* item = new QTableWidgetItem;
		item->setText(str);
		ui.tableWidget_2->setItem(row, 0, item);
	}
}

void DataTableUI::initValues()
{
	mFixedNum = 0;
	mFloatNum = 0;
	mRtdNum = 0;
	mSingleNum = 0;
	mDataSize = 0;
	mMsgNum = 0;
	mDiffSum = 0.0;
}

void DataTableUI::setItemColor(QTableWidget* table,int row, int column, QColor color)
{
	QTableWidgetItem* item = table->item(row, column);
	if (!item) {
		item = new QTableWidgetItem;
		table->setItem(row, column, item);
	}
	item->setBackground(color);
	if (color == Qt::white) {
		item->setForeground(Qt::black);
	}
	else {
		item->setForeground(Qt::white);
	}
}

void DataTableUI::setItemString(QTableWidget* table, int row, QString str)
{
	QTableWidgetItem* item = table->item(row, 1);
	if (item) {
		item->setText(str);
	}
	else {
		item = new QTableWidgetItem;
		table->setItem(row, 1, item);
		item->setText(str);
	}
}

void DataTableUI::setItemValue(QTableWidget* table, int row, double value, int prec)
{
	setItemString(table,row,QString::number(value,'f', prec));
}

void DataTableUI::setItemValue(QTableWidget* table, int row, int value)
{
	setItemString(table,row, QString::number(value));
}

void DataTableUI::setItemValue(QTableWidget* table, int row, uint64_t value)
{
	setItemString(table,row, QString::number(value));
}

void DataTableUI::totalPostType(int posType)
{
	switch (posType)
	{
	case 1:
		mSingleNum++;
		break;
	case 2:
		mRtdNum++;
		break;
	case 5:
		mFloatNum++;
		break;
	case 4:
		mFixedNum++;
		break;
	default:
		break;
	}
}

QString DataTableUI::getPostTypeName(int posType)
{
	QString typeName;
	switch (posType)
	{
	case 1:
		typeName = QString::asprintf("Single (%d)", posType);
		break;
	case 2:
		typeName = QString::asprintf("Rtd (%d)", posType);
		break;
	case 5:
		typeName = QString::asprintf("Float (%d)", posType);
		break;
	case 4:
		typeName = QString::asprintf("Fixed (%d)", posType);
		break;
	default:
		break;
	}
	return typeName;
}

void DataTableUI::onOpen(bool enable)
{
	if (enable) {
		onClearCount();
	}
}

void DataTableUI::onUpdateNMEA(int type) {
	if ($GNGGA == type) {
		nmea_gga_t& m_nmea_gga = DataCache::Instance().m_nmea_gga;
		mMsgNum++;
		mDiffSum += m_nmea_gga.time_diff;
		totalPostType(m_nmea_gga.type);
		setItemValue(ui.tableWidget,0, mFixedNum);
		setItemValue(ui.tableWidget,1, mFloatNum);
		setItemValue(ui.tableWidget, 2, mRtdNum);
		setItemValue(ui.tableWidget, 3, mSingleNum);
		setItemString(ui.tableWidget, 4, m_nmea_gga.nmea_utc);
		setItemValue(ui.tableWidget, 5, m_nmea_gga.time_diff);
		setItemValue(ui.tableWidget, 6, mDiffSum / mMsgNum);
		setItemValue(ui.tableWidget, 7, m_nmea_gga.satid);

		if (m_nmea_gga.type > 0 && m_nmea_gga.type <= 5) {
			setItemColor(ui.tableWidget_2, 0, 1, mPosTypeColor[m_nmea_gga.type - 1]);
			setItemColor(ui.tableWidget_2, 1, 1, mPosTypeColor[m_nmea_gga.type - 1]);
			setItemColor(ui.tableWidget_2, 2, 1, mPosTypeColor[m_nmea_gga.type - 1]);
			setItemColor(ui.tableWidget_2, 3, 1, mPosTypeColor[m_nmea_gga.type - 1]);
		}
		setItemString(ui.tableWidget_2, 0, getPostTypeName(m_nmea_gga.type) + "," + QString::number(m_nmea_gga.sats));
		float height = m_nmea_gga.altitude + m_nmea_gga.undulation;
		BLHCoord blh(D2R(m_nmea_gga.lat), D2R(m_nmea_gga.lon), height);
		xyhCoord xyh = m_crd.BLH2xyh(blh);
		setItemValue(ui.tableWidget_2, 1, xyh.dN, 3);
		setItemValue(ui.tableWidget_2, 2, xyh.dE, 3);
		setItemValue(ui.tableWidget_2, 3, height, 3);
		
	}
	else if ($GPGST == type) {
		nmea_gst_t& m_nmea_gst = DataCache::Instance().m_nmea_gst;
		double HRMS = sqrt(m_nmea_gst.lat_std * m_nmea_gst.lat_std + m_nmea_gst.lon_std * m_nmea_gst.lon_std);
		double VRMS = m_nmea_gst.height_std;
		setItemValue(ui.tableWidget_2, 4, HRMS, 3);
		setItemValue(ui.tableWidget_2, 5, VRMS, 3);
	}
	//else if ($GNHDT == type) {
	//	nmea_hdt_t& nmea_hdt = DataCache::Instance().m_nmea_hdt;
	//	setItemValue(15, nmea_hdt.trueNorthYaw);
	//}else if ($GPVTG == type) {
	//	nmea_vtg_t& nmea_vtg = DataCache::Instance().m_nmea_vtg;
	//	setItemValue(16, nmea_vtg.magneticNorthYaw);
	//}
	else if ($GNHPR == type) {
		nmea_hpr_t& nmea_hpr = DataCache::Instance().m_nmea_hpr;
		setItemValue(ui.tableWidget_2, 6, nmea_hpr.roll);
		setItemValue(ui.tableWidget_2, 7, nmea_hpr.pitch);
		setItemValue(ui.tableWidget_2, 8, nmea_hpr.yaw);
	}
}

void DataTableUI::onDataSize(const int len)
{
	mDataSize += len;
	setItemString(ui.tableWidget, 8, FormatBytes(mDataSize));
}

void DataTableUI::onClearCount()
{
	initValues();
	setItemValue(ui.tableWidget, 0, mFixedNum);
	setItemValue(ui.tableWidget, 1, mFloatNum);
	setItemValue(ui.tableWidget, 2, mRtdNum);
	setItemValue(ui.tableWidget, 3, mSingleNum);
	setItemString(ui.tableWidget, 8, FormatBytes(mDataSize));
}

void DataTableUI::onClearData()
{
	setItemValue(ui.tableWidget, 4, 0.0);
	setItemValue(ui.tableWidget, 5, 0.0);
	setItemValue(ui.tableWidget, 6, 0.0);
	setItemValue(ui.tableWidget, 7, 0);
	setItemValue(ui.tableWidget, 8, 0);
	setItemValue(ui.tableWidget_2,0, 0);
	setItemValue(ui.tableWidget_2,1, 0.0);
	setItemValue(ui.tableWidget_2,2, 0.0);
	setItemValue(ui.tableWidget_2,3, 0.0);
	setItemValue(ui.tableWidget_2,4, 0.0);
	setItemValue(ui.tableWidget_2,5, 0.0);
	setItemValue(ui.tableWidget_2,6, 0.0);
	setItemValue(ui.tableWidget_2,7, 0.0);
	setItemValue(ui.tableWidget_2,8, 0.0);

	setItemColor(ui.tableWidget_2, 0, 1, Qt::white);
	setItemColor(ui.tableWidget_2, 1, 1, Qt::white);
	setItemColor(ui.tableWidget_2, 2, 1, Qt::white);
	setItemColor(ui.tableWidget_2, 3, 1, Qt::white);
}
