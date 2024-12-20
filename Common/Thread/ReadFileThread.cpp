#include "ReadFileThread.h"
#include <QFile>
#include <QDebug>
#include "file_info.h"
#include "DataCache.h"
#include "rtkcmn.h"
#include "gnss_math.h"
#include "gnss_util.h"
#include "kml.h"
#include "nmea.h"
#include "ThreadManager.h"

//long double kahan_summation(const std::vector<double>& data) {
//	long double sum = 0.0;
//	long double c = 0.0; // A running compensation for lost low-order bits.
//	for (double value : data) {
//		long double y = value - c;    // So far, so good: c is zero.
//		long double t = sum + y;      // Alas, sum is big, y small, so low-order digits of y are lost.
//		c = (t - sum) - y;           // (t - sum) cancels the high-order part of y; subtracting y recovers negative (low part of y)
//		sum = t;                     // Algebraically, c should always be zero. Beware eagerly optimising compilers!
//	}
//	return sum;
//}

ReadFileThread::ReadFileThread(QObject* parent)
	: QThread(parent)
	, m_Format(0)
	, m_outputKml(false)
	, m_UseAvg(true)
	, m_delayLimit(10)
	, m_secondOfMinute(60)
	, m_interval(1)
	, m_startSecond(0)
	, m_loopSecond(0)
	, m_breakSecond(0)
	, m_loopCount(0)
	, m_Center(0)
{
	m_ASCII_Decoder = new ASCII_Decoder();
	m_NMEA_Decoder = new NMEA_Decoder();
	m_SPAN_ASCII_Decoder = new SPAN_ASCII_Decoder();
}

ReadFileThread::~ReadFileThread()
{
	delete m_ASCII_Decoder; m_ASCII_Decoder = NULL;
	delete m_NMEA_Decoder; m_NMEA_Decoder = NULL;
	delete m_SPAN_ASCII_Decoder; m_SPAN_ASCII_Decoder = NULL;
}

void ReadFileThread::setFileName(QString name)
{
	m_ReadFileName = name;
}

void ReadFileThread::setFileNameList(QStringList nameList)
{
	m_FileNameList = nameList;
}

void ReadFileThread::setFileFormat(int format)
{
	m_Format = format;
}

void ReadFileThread::setSolType(int type)
{
	m_SolSatusType = type;
}

void ReadFileThread::setKml(bool enable)
{
	m_outputKml = enable;
}

void ReadFileThread::setBaseXYH(double x, double y, double hgt)
{
	m_baseXYH.dE = x;
	m_baseXYH.dN = y;
	m_baseXYH.dU = hgt;
	m_UseAvg = false;
}

void ReadFileThread::setUseAvg()
{
	m_UseAvg = true;
}

void ReadFileThread::setDelayLimit(double delay)
{
	m_delayLimit = delay;
}

void ReadFileThread::setSecondOfMinute(int second)
{
	m_secondOfMinute = second;
}

void ReadFileThread::setInterval(int index)
{
	switch (index)
	{
	case 0:
	case 1:
		m_interval = 1;
		break;
	case 2:
		m_interval = 10;
		break;
	case 3:
		m_interval = 30;
		break;
	case 4:
		m_interval = 60;
		break;
	default:
		m_interval = 1;
		break;
	}
}

void ReadFileThread::setLoopTime(QString start, int loop, int interrupt)
{
	int hour = 0, min = 0;
	double sec = 0.0;
	m_startUTCTime = start;
	time_from_nmea(m_startUTCTime.toLocal8Bit().data(), &hour, &min, &sec);
	hour = ((hour + 24) - 8) % 24;
	m_startSecond = hour * 3600 + min * 60 + (int)sec;
	m_loopSecond = loop;
	m_breakSecond = interrupt;
}

void ReadFileThread::setCenter(double center)
{
	m_Center = center;
}

void ReadFileThread::run()
{
	m_isStop = false;
	if (m_FileNameList.size() > 0) {
		allFixedTimeList.clear();
		for (int i = 0; i < m_FileNameList.size(); i++) {
			setFileName(m_FileNameList.at(i));
			init();
			ThreadManager::Instance().m_ReadFileMutex.lock();
			decode();
			emit sgnFinished(m_ReadFileName);
			qDebug("sgnFinished");
			ThreadManager::Instance().m_ReadFileFlag = false;
			while (!ThreadManager::Instance().m_ReadFileFlag) {
				ThreadManager::Instance().m_ReadFileCondition.wait(&ThreadManager::Instance().m_ReadFileMutex); // 等待条件满足并释放锁
			}
			ThreadManager::Instance().m_ReadFileMutex.unlock();
		}
		writeMultiFixFile();
	}
}

void ReadFileThread::stop()
{
	m_isStop = true;
}

void ReadFileThread::init()
{
	memset(&neuData, 0, sizeof(neuData));
	neuList.clear();
	fixedTimeList.clear();
	DataCache::Instance().m_GGA2ZYZCache.init();
	m_loopCount = 0;
	m_TimeCounter.start();
}

void ReadFileThread::loadFile(QString fileName) {
	//输入文件

	FILE* file = fopen(fileName.toLocal8Bit().data(), "rb");
	if (!file) {
        qDebug() << "open file failed";
		return;
	}
	
	size_t file_size = getFileSize(file);
	size_t read_size = 0;
	size_t readcount = 0;
	char read_cache[READ_CACHE_SIZE] = { 0 };
	m_ASCII_Decoder->init();
	m_NMEA_Decoder->init();
	m_SPAN_ASCII_Decoder->init();
	while (!feof(file)) {
		readcount = fread(read_cache, sizeof(char), READ_CACHE_SIZE, file);
		read_size += readcount;
		for (int i = 0; i < readcount; i++) {
			int len = m_ASCII_Decoder->input_raw(read_cache[i]);
			if (len > 0) {
				parseLine(m_ASCII_Decoder->get_ascii());
			}
			if (m_isStop) {
				break;
			}
		}
		double percent = (double)read_size / (double)file_size * 10000;
		emit sgnProgress((int)percent, m_TimeCounter.elapsed());
		if (m_isStop) {
			break;
		}
	}
	fclose(file);
}

void ReadFileThread::parseLine(char* line)
{
	int ret = 0;
	if (GGA2XYZ_GGA == m_Format || (GGA2XYZ_ALL == m_Format && line[0] == '$')) {
		ret = m_NMEA_Decoder->input_txt_line(line);
		if ($GNGGA == ret) {
			ins_sol_t* ins = m_NMEA_Decoder->getSol();
			neuData.week = ins->wn;
			neuData.seconds = ins->ws;
			nmea_gga_t* gga = m_NMEA_Decoder->getNmeaGGA();
			char str_hour[2] = { 0 };
			strncpy(str_hour, gga->nmea_utc, 2);
			int hour = atoi(str_hour);
			hour = (hour + 8) % 24;
			sprintf(neuData.utcTime, "%02d%s", hour, gga->nmea_utc + 2);
			//strcpy(neuData.utcTime, gga->nmea_utc);
			neuData.lat = gga->lat;
			neuData.lon = gga->lon;
			neuData.hgt = gga->altitude + gga->undulation;
			neuData.type = gga->type;
			neuData.delay = gga->time_diff;
			sprintf(neuData.satid, "%d", gga->satid);
			neuData.sats = gga->sats;
			strcpy(neuData.gstTime, "null");
			strcpy(neuData.hdtTime, "null");
			neuList.push_back(neuData);
			//if (neuList.size() >= 279) {
			neuData.sats = gga->sats;
			//}
		}
		else if ($GPGST == ret) {
			nmea_gst_t* gst = m_NMEA_Decoder->getNmeaGST();
			if (neuList.size() > 0) {
				int last = neuList.size() - 1;
				strcpy(neuList[last].gstTime, gst->nmea_utc);
				neuList[last].HRMS = sqrt(gst->lat_std * gst->lat_std + gst->lon_std * gst->lon_std);
				neuList[last].VRMS = gst->height_std;
			}
		}
		else if ($GNHDT == ret) {
			nmea_hdt_t* hdt = m_NMEA_Decoder->getNmeaHDT();
			if (neuList.size() > 0) {
				int last = neuList.size() - 1;
				neuList[last].yaw = hdt->trueNorthYaw;
			}
		}
	}
	else {
		ret = m_SPAN_ASCII_Decoder->input_txt_line(line);
		if (INSPVAXA == ret && (GGA2XYZ_INSPVAXA == m_Format || GGA2XYZ_ALL == m_Format)) {
			span_header_t* header = m_SPAN_ASCII_Decoder->get_header();
			neuData.week = header->week;
			neuData.seconds = header->gps_seconds;
			inspvax_t* ins = m_SPAN_ASCII_Decoder->get_inspvax();
			gtime_t t = gpst2utc(gpst2time(header->week, header->gps_seconds));
			double ep[6] = { 0 };
			time2epoch(t, ep);
			sprintf(neuData.utcTime, "%02d%02d%05.2f", ((int)ep[3] + 8) % 24, (int)ep[4], ep[5]);
			sprintf(neuData.gstTime, "%02d%02d%05.2f", (int)ep[3], (int)ep[4], ep[5]);
			strcpy(neuData.hdtTime, "null");
			neuData.lat = ins->latitude;
			neuData.lon = ins->longitude;
			neuData.hgt = ins->altitude + ins->undulation;
			neuData.type = ins->position_type;
			strncpy(neuData.satid, "0", 8);
			neuData.HRMS = sqrt(ins->latitude_std * ins->latitude_std + ins->longitude_std * ins->longitude_std);
			neuData.VRMS = ins->altitude_std;
			neuData.yaw = ins->azimuth;
			neuList.push_back(neuData);
		}
		else if (BESTPOSA == ret && (GGA2XYZ_BESTPOSA == m_Format || GGA2XYZ_ALL == m_Format)) {
			span_header_t* header = m_SPAN_ASCII_Decoder->get_header();
			neuData.week = header->week;
			neuData.seconds = header->gps_seconds;
			bestpos_t* bestpos = m_SPAN_ASCII_Decoder->get_bestpos();
			gtime_t t = gpst2utc(gpst2time(header->week, header->gps_seconds));
			double ep[6] = { 0 };
			time2epoch(t, ep);
			sprintf(neuData.utcTime, "%02d%02d%05.2f", ((int)ep[3] + 8) % 24, (int)ep[4], ep[5]);
			sprintf(neuData.gstTime, "%02d%02d%05.2f", (int)ep[3], (int)ep[4], ep[5]);
			strcpy(neuData.hdtTime, "null");
			neuData.lat = bestpos->lat;
			neuData.lon = bestpos->lon;
			neuData.hgt = bestpos->hgt + bestpos->undulation;
			neuData.type = bestpos->position_type;
			neuData.delay = bestpos->diff_age;
			strncpy(neuData.satid, bestpos->stn_id, 8);
			neuData.HRMS = sqrt(bestpos->lat_sigma * bestpos->lat_sigma + bestpos->lon_sigma * bestpos->lon_sigma);
			neuData.VRMS = bestpos->height_sigma;
			neuData.sats = bestpos->SVs;
			neuList.push_back(neuData);
		}
		else if (RTKPOSA == ret && (GGA2XYZ_RTKPOSA == m_Format || GGA2XYZ_ALL == m_Format)) {
			span_header_t* header = m_SPAN_ASCII_Decoder->get_header();
			neuData.week = header->week;
			neuData.seconds = header->gps_seconds;
			rtkpos_t* rtk = m_SPAN_ASCII_Decoder->get_rtkpos();
			gtime_t t = gpst2utc(gpst2time(header->week, header->gps_seconds));
			double ep[6] = { 0 };
			time2epoch(t, ep);
			sprintf(neuData.utcTime, "%02d%02d%05.2f", ((int)ep[3] + 8) % 24, (int)ep[4], ep[5]);
			sprintf(neuData.gstTime, "%02d%02d%05.2f", (int)ep[3], (int)ep[4], ep[5]);
			strcpy(neuData.hdtTime, "null");
			neuData.lat = rtk->lat;
			neuData.lon = rtk->lon;
			neuData.hgt = rtk->hgt + rtk->undulation;
			neuData.type = rtk->position_type;
			neuData.delay = rtk->diff_age;
			strncpy(neuData.satid, rtk->stn_id, 8);
			neuData.HRMS = sqrt(rtk->lat_sigma * rtk->lat_sigma + rtk->lon_sigma * rtk->lon_sigma);
			neuData.VRMS = rtk->height_sigma;
			neuData.sats = rtk->SVs;
			neuList.push_back(neuData);
		}
		else if (PPPNAVA == ret && (GGA2XYZ_PPPNAVA == m_Format || GGA2XYZ_ALL == m_Format)) {
			span_header_t* header = m_SPAN_ASCII_Decoder->get_header();
			neuData.week = header->week;
			neuData.seconds = header->gps_seconds;
			pppnav_t* ppp = m_SPAN_ASCII_Decoder->get_pppnav();
			gtime_t t = gpst2utc(gpst2time(header->week, header->gps_seconds));
			double ep[6] = { 0 };
			time2epoch(t, ep);
			sprintf(neuData.utcTime, "%02d%02d%05.2f", ((int)ep[3] + 8) % 24, (int)ep[4], ep[5]);
			sprintf(neuData.gstTime, "%02d%02d%05.2f", (int)ep[3], (int)ep[4], ep[5]);
			strcpy(neuData.hdtTime, "null");
			neuData.lat = ppp->lat;
			neuData.lon = ppp->lon;
			neuData.hgt = ppp->hgt + ppp->undulation;
			neuData.type = ppp->position_type;
			neuData.delay = ppp->diff_age;
			strncpy(neuData.satid, ppp->stn_id, 8);
			neuData.HRMS = sqrt(ppp->lat_sigma * ppp->lat_sigma + ppp->lon_sigma * ppp->lon_sigma);
			neuData.VRMS = ppp->height_sigma;
			neuData.sats = ppp->SVs;
			neuList.push_back(neuData);
		}
	}
}

void ReadFileThread::decode()
{
	//加载文件
	loadFile(m_ReadFileName);

	//解码输出文件路径
	QString outputFileName = m_ReadFileName.left(m_ReadFileName.lastIndexOf('.')) + ".dat.csv";
	QFile output(outputFileName);
	if (!output.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "open output file failed";
		return;
	}
	//输出格式
	output.write("local time,北坐标,东坐标,高程,解状态,延迟,基站ID,GSTTIME,HRMS,VRMS,卫星颗数,HDT时间,YAW,△x,△y,△p,△h\n");

	if (m_UseAvg) {
		double sum_lat = 0.0;
		double sum_lon = 0.0;
		double sum_hgt = 0.0;
		int sumCount = 0;
		int typeSort[6] = {4,3,5,2,1,0};
		for (int t = 0; t < 6; t++) {
			for (int i = 0; i < neuList.size(); i++) {
				if (neuList[i].type != typeSort[t])
					continue;
				sum_lat += neuList[i].lat;
				sum_lon += neuList[i].lon;
				sum_hgt += neuList[i].hgt;
				sumCount++;
			}
			if (sumCount > 0) {
				break;
			}
		}
		m_baseBLH.dLat = sum_lat / sumCount;
		m_baseBLH.dLon = sum_lon / sumCount;
		m_baseBLH.dHgt = sum_hgt / sumCount;
		BLHCoord avgBLH(D2R(m_baseBLH.dLat),D2R(m_baseBLH.dLon), m_baseBLH.dHgt);
		m_baseXYH = m_crd.BLH2xyh(avgBLH, D2R(m_Center));
	}
	DataCache::Instance().m_GGA2ZYZCache.baseXYH = m_baseXYH;

	QString kmlFileName = m_ReadFileName.left(m_ReadFileName.lastIndexOf('.')) + ".kml";
	FILE* kml = NULL;
	if (m_outputKml) {
		kml = fopen(kmlFileName.toLocal8Bit().data(), "w");
		print_kml_header(kml);
	}
	QList<QPointF> signalList;
	QList<QPointF> dGPSList;
	QList<QPointF> floatList;
	QList<QPointF> fixedList;
	QList<QPointF> insList;
	//转换成ENU
	int count = 0;
	int second = 0;
	for (int i = 0; i < neuList.size(); i++) {
		double percent = (double)(i + 1) / (double)neuList.size() * 10000;
		emit sgnProgress((int)percent, m_TimeCounter.elapsed());

		second = (int)neuList[i].seconds;
		//固定周期
		if (m_startSecond > 0 && m_loopSecond > 0) {
			while (second >= m_startSecond + (m_loopCount + 1) * m_loopSecond) {
				m_loopCount++;
			}
			if (second >= m_startSecond + m_breakSecond + m_loopCount * m_loopSecond && second < m_startSecond + (m_loopCount + 1) * m_loopSecond) {
				if (neuList[i].type == 4) {
					int fixedTime = second - (m_startSecond + m_breakSecond + m_loopCount * m_loopSecond);
					fixedTimeList.push_back(fixedTime);
					m_loopCount++;
				}
				else {
					if (second == m_startSecond + (m_loopCount + 1) * m_loopSecond - 1) {
						fixedTimeList.push_back(-1);
						m_loopCount++;
					}
				}
			}
		}

		if (neuList[i].type == 0) {//未定位
			output.write("\n");
			continue;
		}
		if (m_SolSatusType == FIXED_TYPE_SOL && neuList[i].type != 4) {//选择fixed选项
			output.write("\n");
			continue;
		}
		if (neuList[i].delay > m_delayLimit) {
			output.write("\n");
			continue;
		}
		if (second % 60 > m_secondOfMinute) {
			output.write("\n");
			continue;
		}
		if (m_interval > 1) {
			if (second % m_interval != 0) {
				output.write("\n");
				continue;
			}
		}
		count++;
		BLHCoord BLH(D2R(neuList[i].lat), D2R(neuList[i].lon), neuList[i].hgt);
		xyhCoord xyh = m_crd.BLH2xyh(BLH,D2R(m_Center));
		neuList[i].E = xyh.dE;
		neuList[i].N = xyh.dN;
		neuList[i].U = xyh.dU;
		double deltaE = neuList[i].E - m_baseXYH.dE;
		double deltaN = neuList[i].N - m_baseXYH.dN;
		double deltaP = sqrt(deltaE * deltaE + deltaN * deltaN);
		double deltaH = neuList[i].U - m_baseXYH.dU;
		//写csv文件
		//output.write(QString::asprintf("%s,%12.3f,%12.3f,%7.2f,%2d,%6.2f,%8s,%10s,%8.3f,%8.3f,%3d,%10s,%7.2f,%10.4f,%10.4f,%10.4f,%10.4f\n", neuList[i].utcTime, neuList[i].N, neuList[i].E, neuList[i].U, neuList[i].type, neuList[i].delay, neuList[i].satid, neuList[i].gstTime, neuList[i].HRMS, neuList[i].VRMS, neuList[i].sats, neuList[i].hdtTime, neuList[i].yaw, deltaN, deltaE, deltaP, deltaH).toLocal8Bit());
		output.write(QString::asprintf("%s,%.3f,%.3f,%.3f,%d,%.2f,%s,%s,%.3f,%.3f,%02d,%s,%s,%.4f,%.4f,%.4f,%.4f\n", neuList[i].utcTime, neuList[i].N, neuList[i].E, neuList[i].U, neuList[i].type, neuList[i].delay, neuList[i].satid, neuList[i].gstTime, neuList[i].HRMS, neuList[i].VRMS, neuList[i].sats, neuList[i].hdtTime, "", deltaN, deltaE, deltaP, deltaH).toLocal8Bit());
		//写kml文件
		if (kml) {
			print_kml_gga(kml, neuList[i].lat, neuList[i].lon, neuList[i].hgt, neuList[i].type, neuList[i].week, neuList[i].seconds, neuList[i].yaw,NULL);
		}

		double E = neuList[i].E - m_baseXYH.dE;
		double N = neuList[i].N - m_baseXYH.dN;
		DataCache::Instance().m_GGA2ZYZCache.minX = min(DataCache::Instance().m_GGA2ZYZCache.minX, (double)E);
		DataCache::Instance().m_GGA2ZYZCache.maxX = max(DataCache::Instance().m_GGA2ZYZCache.maxX, (double)E);
		DataCache::Instance().m_GGA2ZYZCache.minY = min(DataCache::Instance().m_GGA2ZYZCache.minY, (double)N);
		DataCache::Instance().m_GGA2ZYZCache.maxY = max(DataCache::Instance().m_GGA2ZYZCache.maxY, (double)N);
		if (neuList[i].type == 1) {
			signalList.append(QPointF(E, N));
		}
		else if (neuList[i].type == 2) {
			dGPSList.append(QPointF(E, N));
		}
		else if (neuList[i].type == 5) {
			floatList.append(QPointF(E, N));
		}
		else if (neuList[i].type == 4) {
			fixedList.append(QPointF(E, N));
		}
		else if (neuList[i].type == 3) {
			insList.append(QPointF(E, N));
		}
		double U = neuList[i].U - m_baseXYH.dU;
		if(DataCache::Instance().m_GGA2ZYZCache.minT == 0.0)
			DataCache::Instance().m_GGA2ZYZCache.minT = neuList[i].seconds;
		DataCache::Instance().m_GGA2ZYZCache.maxT = neuList[i].seconds;
		DataCache::Instance().m_GGA2ZYZCache.minH = min(DataCache::Instance().m_GGA2ZYZCache.minH, (double)U);
		DataCache::Instance().m_GGA2ZYZCache.maxH = max(DataCache::Instance().m_GGA2ZYZCache.maxH, (double)U);
		DataCache::Instance().m_GGA2ZYZCache.HValueList.append(QPointF(neuList[i].seconds, U));
	}
	DataCache::Instance().m_GGA2ZYZCache.fixedTimeList.append(fixedTimeList);
	DataCache::Instance().m_GGA2ZYZCache.signalCount = signalList.size();
    DataCache::Instance().m_GGA2ZYZCache.dGPSCount = dGPSList.size();
	DataCache::Instance().m_GGA2ZYZCache.floatCount = floatList.size();
	DataCache::Instance().m_GGA2ZYZCache.fixedCount = fixedList.size();
	DataCache::Instance().m_GGA2ZYZCache.insCount = insList.size();
	if (signalList.size() >= 2000) {
		int setp = signalList.size() / 1000;
		for (int i = 0; i < signalList.size(); i += setp) {
			DataCache::Instance().m_GGA2ZYZCache.signalList.append(signalList[i]);
		}
	}
	else {
        DataCache::Instance().m_GGA2ZYZCache.signalList.swap(signalList);
	}
	if (dGPSList.size() >= 2000) {
		int setp = dGPSList.size() / 1000;
		for (int i = 0; i < dGPSList.size(); i += setp) {
			DataCache::Instance().m_GGA2ZYZCache.dGPSList.append(dGPSList[i]);
		}
	}
	else {
        DataCache::Instance().m_GGA2ZYZCache.dGPSList.swap(dGPSList);
	}
	if (floatList.size() >= 2000) {
		int setp = floatList.size() / 1000;
		for (int i = 0; i < floatList.size(); i += setp) {
			DataCache::Instance().m_GGA2ZYZCache.floatList.append(floatList[i]);
		}
	}
	else {
        DataCache::Instance().m_GGA2ZYZCache.floatList.swap(floatList);
	}
	if (fixedList.size() >= 2000) {
		int setp = fixedList.size() / 1000;
		for (int i = 0; i < fixedList.size(); i += setp) {
			DataCache::Instance().m_GGA2ZYZCache.fixedList.append(fixedList[i]);
		}
	}
	else {
        DataCache::Instance().m_GGA2ZYZCache.fixedList.swap(fixedList);
	}
	if (insList.size() >= 2000) {
		int setp = insList.size() / 1000;
		for (int i = 0; i < insList.size(); i += setp) {
			DataCache::Instance().m_GGA2ZYZCache.insList.append(insList[i]);
		}
	}
	else {
        DataCache::Instance().m_GGA2ZYZCache.insList.swap(insList);
	}

	if (kml) {
		print_kml_eof(kml);
		fclose(kml); kml = NULL;
	}
	output.close();
	writeSingleFixFile();
}

void ReadFileThread::writeSingleFixFile()
{
	if (m_startSecond > 0 && m_loopSecond > 0) {
		//保存文件
		QString fixTimeFileName = m_ReadFileName.left(m_ReadFileName.lastIndexOf('.')) + "-AR-FixTime.csv";
		QFile fixTimeFile(fixTimeFileName);
		if (!fixTimeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			return;
		}
		//获取m_ReadFileName文件名
		QString ReadFileName = m_ReadFileName.right(m_ReadFileName.length() - m_ReadFileName.lastIndexOf('/') - 1);
		fixTimeFile.write(ReadFileName.toLocal8Bit() + "\n");
		for (int i = 0; i < fixedTimeList.size(); i++) {
			if (fixedTimeList[i] == -1) {
				fixTimeFile.write("未固定\n");
			}
			else {
				fixTimeFile.write(QString::number(fixedTimeList[i]).toLocal8Bit() + "\n");
			}
		}
		fixTimeFile.close();
		allFixedTimeList.insert(ReadFileName, fixedTimeList);
	}
}

void ReadFileThread::writeMultiFixFile()
{
	if (allFixedTimeList.size() > 0) {
		int maxLength = 0;
		for (auto it = allFixedTimeList.begin(); it != allFixedTimeList.end(); it++) {
			if (it.value().size() > maxLength) {
				maxLength = it.value().size();
			}
		}
		//保存文件
        QString fixTimeFileName = m_ReadFileName.left(m_ReadFileName.lastIndexOf('/')) + "/AR-FixTime.csv";
        QFile fixTimeFile(fixTimeFileName);
		if (!fixTimeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			return;
		}
		//写ReadFileName文件名
		for (auto it = allFixedTimeList.begin(); it != allFixedTimeList.end(); it++) {
			fixTimeFile.write(it.key().toLocal8Bit());
			if (it != allFixedTimeList.end() - 1) {
				fixTimeFile.write(",");
			}
			else {
				fixTimeFile.write("\n");
			}
		}
		for (int i = 0; i < maxLength; i++) {
			for (auto it = allFixedTimeList.begin(); it != allFixedTimeList.end(); it++) {
				if (i < it.value().size()) {
					if (it.value()[i] == -1) {
						fixTimeFile.write("未固定");
					}
					else {
						fixTimeFile.write(QString::number(it.value()[i]).toLocal8Bit());
					}
					if (it != allFixedTimeList.end() - 1) {
						fixTimeFile.write(",");
					}
					else {
						fixTimeFile.write("\n");
					}
				}
			}
		}
	}
}
