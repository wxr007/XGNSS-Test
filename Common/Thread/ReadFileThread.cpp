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

//QXlsx
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxchartsheet.h"
#include "xlsxdocument.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
QXLSX_USE_NAMESPACE

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
	//memset(&neuData, 0, sizeof(neuData));
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
//utc时间秒数做四舍五入 开始
static void utctime_round(char* nmea_utc) {
	char new_utc[10] = { 0 };
	char nmea_utc_second[6] = { 0 };
	strncpy(nmea_utc_second, nmea_utc + 4, 5);
	int second = (int)(atof(nmea_utc_second) + 0.5);
	sprintf(nmea_utc_second, "%02d.00", second);
	strncpy(new_utc, nmea_utc, 4);
	strncpy(new_utc + 4, nmea_utc_second, 5);
    strcpy(nmea_utc, new_utc);
}

void ReadFileThread::fillWithGGA(NEUCSV& neudata,ins_sol_t* ins, nmea_gga_t* gga) {
	neudata.week = ins->wn;
	neudata.seconds = ins->ws;
	strcpy(neudata.ggaTime, gga->nmea_utc);
	utc2cst(neudata.utcTime, gga->nmea_utc);
	//char str_hour[2] = { 0 };
	//strncpy(str_hour, gga->nmea_utc, 2);
	//int hour = atoi(str_hour);
	//hour = (hour + 8) % 24;
	//sprintf(neudata.utcTime, "%02d%s", hour, gga->nmea_utc + 2);
	neudata.lat = gga->lat;
	neudata.lon = gga->lon;
	neudata.hgt = gga->altitude + gga->undulation;
	neudata.type = gga->type;
	neudata.delay = gga->time_diff; 
	sprintf(neudata.satid, "%d", gga->satid);
	neudata.sats = gga->sats;
}

void ReadFileThread::utc2cst(char* cst_time, char* utc_time) {
	char str_hour[3] = { 0 };
	strncpy(str_hour, utc_time, 2);
	int hour = atoi(str_hour);
	hour = (hour + 8) % 24;
	sprintf(cst_time, "%02d%s", hour, utc_time + 2);
}

void ReadFileThread::parseLine(char* line)
{
	int ret = 0;
	NEUCSV neuData = { 0 };
	//memset(&neuData, 0, sizeof(neuData));
	if (GGA2XYZ_GGA == m_Format || (GGA2XYZ_ALL == m_Format && line[0] == '$')) {
		ret = m_NMEA_Decoder->input_txt_line(line);
		if ($GNGGA == ret) {
			ins_sol_t* ins = m_NMEA_Decoder->getSol();
			nmea_gga_t* gga = m_NMEA_Decoder->getNmeaGGA();
			if(strcmp(gga->nmea_utc+6, ".00") != 0){
				utctime_round(gga->nmea_utc);
				gga->ws = lrintf(gga->ws);
			}			
			//strcpy(neuData.gstTime, "");
			if (neuList.size() > 0) {
				int last = neuList.size() - 1;
				if (gga->ws <= neuList[last].seconds || gga->ws < neuList[last].hdt_ws) {
					return;
				}
				if (strcmp(neuList[last].hdtTime, gga->nmea_utc) == 0) {
					fillWithGGA(neuList[last], ins, gga);
				}
				else {
					fillWithGGA(neuData, ins, gga);
                    neuList.push_back(neuData);
				}
			}
			else {
				fillWithGGA(neuData, ins, gga);
                neuList.push_back(neuData);
			}				
			DataCache::Instance().m_GGA2ZYZCache.m_nGGACountAll++;
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
				neuList[last].has_hpr = 1;
				neuList[last].yaw = hdt->trueNorthYaw;
				//sprintf(neuList[last].yaw_str, "%.3f", hdt->trueNorthYaw);
			}
		}
		else if ($GNHPR == ret) {
            nmea_hpr_t* hpr = m_NMEA_Decoder->getNmeaHPR();
			//strcpy(neuData.hdtTime, "");
			if (neuList.size() > 0) {
				int last = neuList.size() - 1;
				if (strcmp(neuList[last].ggaTime, hpr->nmea_utc) == 0) {
					strcpy(neuList[last].hdtTime, hpr->nmea_utc);
					if (hpr->type == 4) {
						neuList[last].has_hpr = 1;
						neuList[last].yaw = hpr->yaw;
						neuList[last].pitch = hpr->pitch;
						//sprintf(neuList[last].yaw_str, "%.3f", hpr->yaw);
					}
				}
				else {
					strcpy(neuData.hdtTime, hpr->nmea_utc);
					if (hpr->type == 4) {
						neuData.has_hpr = 1;
						neuData.yaw = hpr->yaw;
                        neuData.pitch = hpr->pitch;
						//sprintf(neuData.yaw_str, "%.3f", hpr->yaw);
					}
					neuList.push_back(neuData);
				}
			}
			else {
				strcpy(neuData.hdtTime, hpr->nmea_utc);
				if (hpr->type == 4) {
                    neuData.has_hpr = 1;
					neuData.yaw = hpr->yaw;
                    neuData.pitch = hpr->pitch;
					//sprintf(neuData.yaw_str, "%.3f", hpr->yaw);
				}
				neuList.push_back(neuData);
			}
			DataCache::Instance().m_GGA2ZYZCache.m_nHPRCountAll++;
		}
		else if ($PSAT_HPR == ret) {
			nmea_hpr_t* hpr = m_NMEA_Decoder->getNmeaHPR();
			//strcpy(neuData.hdtTime, "");
			if (neuList.size() > 0) {
				int last = neuList.size() - 1;
				if (strcmp(neuList[last].ggaTime, hpr->nmea_utc) == 0) {
					strcpy(neuList[last].hdtTime, hpr->nmea_utc);
					neuList[last].hdt_ws = hpr->ws;
					if (hpr->type == 4) {
                        neuList[last].has_hpr = 1;
						neuList[last].yaw = hpr->yaw;
						neuList[last].pitch = hpr->pitch;
                        //sprintf(neuList[last].yaw_str, "%.3f", hpr->yaw);
					}
				}
				else {
					strcpy(neuData.hdtTime, hpr->nmea_utc);
					neuData.hdt_ws = hpr->ws;
					if (hpr->type == 4) {
                        neuData.has_hpr = 1;
						neuData.yaw = hpr->yaw;
                        neuData.pitch = hpr->pitch;
                        //sprintf(neuData.yaw_str, "%.3f", hpr->yaw);
					}
					neuList.push_back(neuData);
				}
			}
			else {
				strcpy(neuData.hdtTime, hpr->nmea_utc);
				neuData.hdt_ws = hpr->ws;
				if (hpr->type == 4) {
                    neuData.has_hpr = 1;
					neuData.yaw = hpr->yaw;
                    neuData.pitch = hpr->pitch;
                    //sprintf(neuData.yaw_str, "%.3f", hpr->yaw);
				}
				neuList.push_back(neuData);
			}
			DataCache::Instance().m_GGA2ZYZCache.m_nHPRCountAll++;
		}
		else if ($GNRMC == ret) {
			nmea_rmc_t* rmc = m_NMEA_Decoder->getNmeaRMC();
            if (neuList.size() > 0) {
				int last = neuList.size() - 1;
                strcpy(neuList[last].rmcTime, rmc->nmea_utc);
				neuList[last].rmcVelocity = rmc->velocity;
				neuList[last].rmcCourse = rmc->course_over_ground;
				neuList[last].rmcStatus = rmc->status;
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
			//strcpy(neuData.hdtTime, "");
			neuData.lat = ins->latitude;
			neuData.lon = ins->longitude;
			neuData.hgt = ins->altitude + ins->undulation;
			neuData.type = ins->position_type;
			strncpy(neuData.satid, "0", 8);
			neuData.HRMS = sqrt(ins->latitude_std * ins->latitude_std + ins->longitude_std * ins->longitude_std);
			neuData.VRMS = ins->altitude_std;
			neuData.has_hpr = 1;
			neuData.yaw = ins->azimuth;
            neuData.pitch = ins->pitch;            
            //sprintf(neuData.yaw_str, "%.3f", ins->azimuth);
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
			//strcpy(neuData.hdtTime, "");
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
			//strcpy(neuData.hdtTime, "");
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
			//strcpy(neuData.hdtTime, "");
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
	//QString outputFileName = m_ReadFileName.left(m_ReadFileName.lastIndexOf('.')) + ".dat.csv";
	//QFile output(outputFileName);
	//if (!output.open(QIODevice::WriteOnly | QIODevice::Text)) {
	//	qDebug() << "open output file failed";
	//	return;
	//}
	QString xlsxFileName = m_ReadFileName.left(m_ReadFileName.lastIndexOf('.')) + ".dat.xlsx";
	Document xlsx;
	//第一个sheet
	xlsx.addSheet("GGA");	
	xlsx.write(1,1, u8"local time");
    xlsx.write(1,2, u8"北坐标");
    xlsx.write(1,3, u8"东坐标");
    xlsx.write(1,4, u8"高程");
    xlsx.write(1,5, u8"解状态");
    xlsx.write(1,6, u8"差分龄期");
    xlsx.write(1,7, u8"基站ID");
    xlsx.write(1,8, u8"GSTTIME");
    xlsx.write(1,9, u8"HRMS");
    xlsx.write(1,10, u8"VRMS");
    xlsx.write(1,11, u8"卫星颗数");
    xlsx.write(1,12, u8"△x");
    xlsx.write(1,13, u8"△y");
    xlsx.write(1,14, u8"△p");
    xlsx.write(1,15, u8"△h");
	//第二个sheet
	xlsx.addSheet("HPR");
	xlsx.write(1,1, u8"HPR时间");
    xlsx.write(1,2, u8"航向角");
    xlsx.write(1,3, u8"俯仰角");
	//第三个sheet
	xlsx.addSheet("RMC");
    xlsx.write(1,1, u8"RMC时间");
    xlsx.write(1,2, u8"速度值");
    xlsx.write(1,3, u8"对地航向");
	//输出格式
	//output.write("local time,北坐标,东坐标,高程,解状态,延迟,基站ID,GSTTIME,HRMS,VRMS,卫星颗数,HDT时间,YAW,△x,△y,△p,△h,RMC时间,速度值,对地航向\n");

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
			if (sumCount > 0) {//只统计精确度最高的类型
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
	//计算航向角均值和俯仰角平均值
	{
		DataCache::Instance().m_GGA2ZYZCache.m_avgYaw = 0.0;
        DataCache::Instance().m_GGA2ZYZCache.m_avgPitch = 0.0;
		double sum_yaw = 0.0;
		double sum_pitch = 0.0;
        int sumCount = 0;
        for (int i = 0; i < neuList.size(); i++) {
            if (neuList[i].yaw == 0 && neuList[i].pitch == 0)
                continue;
            sum_yaw += neuList[i].yaw;
            sum_pitch += neuList[i].pitch;
            sumCount++;
        }
        if (sumCount > 0) {
			DataCache::Instance().m_GGA2ZYZCache.m_avgYaw = sum_yaw / sumCount;
            DataCache::Instance().m_GGA2ZYZCache.m_avgPitch = sum_pitch / sumCount;
        }
	}

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
	int second = 0;
	double max_delay = DBL_MIN;
    double min_delay = DBL_MAX;
	double sum_delay = 0.0;
	for (int i = 0; i < neuList.size(); i++) {
		double percent = (double)(i + 1) / (double)neuList.size() * 10000;
		emit sgnProgress((int)percent, m_TimeCounter.elapsed());
		NEUCSV& neuData = neuList[i];

		if ((int)neuList[i].seconds != 0) {
			second = (int)neuList[i].seconds;
		}
		else if ((int)neuList[i].hdt_ws != 0) {
			second = (int)neuList[i].hdt_ws;
		}
		
		//判断是否要计算固定周期，如果m_startSecond和m_loopSecond都大于0，则说明需要计算固定周期
		if (m_startSecond > 0 && m_loopSecond > 0) {
			//推算到当前的循环周期,如果当前时间大于整个周期的结束时间就跳到下一个周期
			while (second >= m_startSecond + (m_loopCount + 1) * m_loopSecond) {
				m_loopCount++;
			}
			//如果时间大于中断时间，并且在周期内，则计算固定时间
			if (second >= m_startSecond + m_breakSecond + m_loopCount * m_loopSecond && second < m_startSecond + (m_loopCount + 1) * m_loopSecond) {
				//如果是固定解，则计算到达固定的时间，并且进入下一个循环，等待时间进入循环内。
				if (neuList[i].type == 4) {
					int fixedTime = second - (m_startSecond + m_breakSecond + m_loopCount * m_loopSecond);
					fixedTimeList.push_back(fixedTime);
					m_loopCount++;
				}
				else {
					//如果当前不是固定解，则判断是否到达周期结束时间，如果是，则在列表中插入未固定结果，并且进入下一个循环，否则继续等待
					if (second == m_startSecond + (m_loopCount + 1) * m_loopSecond - 1) {
						fixedTimeList.push_back(-1);
						m_loopCount++;
					}
				}
			}
		}
		//sum_delay += neuData.delay;
		if (m_SolSatusType == FIXED_TYPE_SOL) {
			if (neuData.type == 4) {
				max_delay = max_delay > neuData.delay ? max_delay : neuData.delay;
				min_delay = min_delay < neuData.delay ? min_delay : neuData.delay;
			}
		}
		else {
			max_delay = max_delay > neuData.delay ? max_delay : neuData.delay;
			min_delay = min_delay < neuData.delay ? min_delay : neuData.delay;
		}
		//QString gga_str = " , , , , , ,";
		//QString gst_str = " , , , ,";
		//QString hpr_str = " , ,";
		//QString delta_str = " , , , ,";
		//QString rmc_str = " , , , ";

		//“中断时间”这段时间内，GGA,RMC和HRP即使有值，也不要输出出来，这段时间内的数据可直接输出空值。
		if (m_startSecond > 0 && m_loopSecond > 0 && second > m_startSecond) {
			//如果时间在周期的中断时间内，则输出空行并且跳过
			if (second > m_startSecond + m_loopCount * m_loopSecond && second <= m_startSecond + m_breakSecond + m_loopCount * m_loopSecond){
				//output.write("\n");
				continue;
			}
		}
		if (neuList[i].delay > m_delayLimit) {
			//output.write("\n");
			//continue;
		}
		else if (second % 60 > m_secondOfMinute) {
			//output.write("\n");
			//continue;
		}
		else if (m_interval > 1 && second % m_interval != 0) {
			//output.write("\n");
			//continue;
		}
		else if (neuList[i].type == 0 || m_SolSatusType == FIXED_TYPE_SOL && neuList[i].type != 4) {//未定位或者选择fixed选项
			//gga_str = QString::asprintf("%s, , , , , , ,", neuList[i].utcTime);			
		}
		else {
			BLHCoord BLH(D2R(neuList[i].lat), D2R(neuList[i].lon), neuList[i].hgt);
			xyhCoord xyh = m_crd.BLH2xyh(BLH, D2R(m_Center));
			neuList[i].E = xyh.dE;
			neuList[i].N = xyh.dN;
			neuList[i].U = xyh.dU;
			double deltaE = neuList[i].E - m_baseXYH.dE;
			double deltaN = neuList[i].N - m_baseXYH.dN;
			double deltaP = sqrt(deltaE * deltaE + deltaN * deltaN);
			double deltaH = neuList[i].U - m_baseXYH.dU;
			//写csv文件
			//gga_str = QString::asprintf("%s,%.3f,%.3f,%.3f,%d,%.2f,%s",
			//	neuList[i].utcTime, neuList[i].N, neuList[i].E, neuList[i].U, neuList[i].type, neuList[i].delay, neuList[i].satid);
			//gst_str = QString::asprintf(" ,%s,%.3f,%.3f,%02d", neuList[i].gstTime, neuList[i].HRMS, neuList[i].VRMS, neuList[i].sats);
			//delta_str = QString::asprintf(" ,%.4f,%.4f,%.4f,%.4f", deltaN, deltaE, deltaP, deltaH);

			xlsx.selectSheet("GGA");
            xlsx.write(i+2, 1, neuList[i].utcTime);
            xlsx.write(i+2, 2, neuList[i].N);
            xlsx.write(i+2, 3, neuList[i].E);
            xlsx.write(i+2, 4, neuList[i].U);
            xlsx.write(i+2, 5, neuList[i].type);
            xlsx.write(i+2, 6, neuList[i].delay);
            xlsx.write(i+2, 7, neuList[i].satid);
			xlsx.write(i+2, 8, neuList[i].gstTime);
            xlsx.write(i+2, 9, neuList[i].HRMS);
            xlsx.write(i+2, 10, neuList[i].VRMS);
            xlsx.write(i+2, 11, neuList[i].sats);
            xlsx.write(i+2, 12, deltaN);
            xlsx.write(i+2, 13, deltaE);
			xlsx.write(i+2, 14, deltaP);
			xlsx.write(i+2, 15, deltaH);

			//写kml文件
			if (kml) {
				print_kml_gga(kml, neuList[i].lat, neuList[i].lon, neuList[i].hgt, neuList[i].type, neuList[i].week, neuList[i].seconds, neuList[i].yaw, NULL);
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
			if (DataCache::Instance().m_GGA2ZYZCache.minT == 0.0)
				DataCache::Instance().m_GGA2ZYZCache.minT = neuList[i].seconds;
			DataCache::Instance().m_GGA2ZYZCache.maxT = neuList[i].seconds;
			DataCache::Instance().m_GGA2ZYZCache.minH = min(DataCache::Instance().m_GGA2ZYZCache.minH, (double)U);
			DataCache::Instance().m_GGA2ZYZCache.maxH = max(DataCache::Instance().m_GGA2ZYZCache.maxH, (double)U);
			DataCache::Instance().m_GGA2ZYZCache.HValueList.append(QPointF(neuList[i].seconds, U));
		}
		if (strlen(neuList[i].hdtTime) > 0) {
			//hpr_str = QString::asprintf(" ,%s,%.3f", neuList[i].hdtTime, neuList[i].yaw);
			xlsx.selectSheet("HPR");
			char cstTime[10] = { 0 };
			utc2cst(cstTime, neuList[i].hdtTime);
			xlsx.write(i + 2, 1, cstTime);
			if (neuList[i].has_hpr) {
				xlsx.write(i + 2, 2, neuList[i].yaw);
				xlsx.write(i + 2, 3, neuList[i].pitch);
			}
		}
		if (neuList[i].rmcStatus == 'S') {
			//rmc_str = QString::asprintf(",%s,%f,%f", neuList[i].rmcTime, neuList[i].rmcVelocity, neuList[i].rmcCourse);
            xlsx.selectSheet("RMC");
			char cstTime[10] = { 0 };
			utc2cst(cstTime, neuList[i].rmcTime);
            xlsx.write(i+2, 1, cstTime);
            xlsx.write(i+2, 2, neuList[i].rmcVelocity);
            xlsx.write(i+2, 3, neuList[i].rmcCourse);
		}
		DataCache::Instance().m_GGA2ZYZCache.m_nHPRCountFixed++;
		//output.write((gga_str + gst_str + hpr_str + delta_str + rmc_str + "\n").toLocal8Bit());
	}
	//循环结束
	DataCache::Instance().m_GGA2ZYZCache.fixedTimeList.append(fixedTimeList);
	DataCache::Instance().m_GGA2ZYZCache.signalCount = signalList.size();
    DataCache::Instance().m_GGA2ZYZCache.dGPSCount = dGPSList.size();
	DataCache::Instance().m_GGA2ZYZCache.floatCount = floatList.size();
	DataCache::Instance().m_GGA2ZYZCache.fixedCount = fixedList.size();
	DataCache::Instance().m_GGA2ZYZCache.insCount = insList.size();
	//DataCache::Instance().m_GGA2ZYZCache.avgDelay = sum_delay / neuList.size();
    DataCache::Instance().m_GGA2ZYZCache.maxDelay = max_delay;
    DataCache::Instance().m_GGA2ZYZCache.minDelay = min_delay;
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
	xlsx.selectSheet(0);
	xlsx.saveAs(xlsxFileName);
	//output.close();
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
