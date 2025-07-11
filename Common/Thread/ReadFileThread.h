#pragma once

#include <QThread>
#include <QVector>
#include <QMap>
#include <QElapsedTimer>
#include "ASCII\ASCII_Decoder.h"
#include "NMEA\NMEA_Decoder.h"
#include "SPAN\SPAN_ASCII_Decoder.h"
#include "Coordinate.h"

struct NEUCSV {
	char utcTime[10];
	char ggaTime[10];
	uint16_t week;
	double seconds;
	double lat;
	double lon;
	float hgt;
	double N;
	double E;
	float U;
	int type;
	float delay;
	char satid[8];
	char gstTime[10];
	float HRMS;
	float VRMS;
	int sats;
	int8_t has_hpr;
	char hdtTime[10];
	double hdt_ws;
	float yaw;
	float pitch;
    char yaw_str[10];
	float deltaX;
	float deltaY;
	float deltaP;
	float deltaH;
	char rmcTime[10];
	float rmcVelocity;
	float rmcCourse;//对地航向
	char rmcStatus; //S=安全; C=警告; U=不安全; V=导航无效
};

class ReadFileThread  : public QThread
{
	Q_OBJECT
enum {
		GGA2XYZ_ALL,
		GGA2XYZ_GGA,
		GGA2XYZ_PPPNAVA,
		GGA2XYZ_BESTPOSA,
		GGA2XYZ_INSPVAXA,
		GGA2XYZ_RTKPOSA,
	};
enum {
		FIXED_TYPE_SOL,
		ALL_TYPE_SOL,
};
public:
	ReadFileThread(QObject *parent);
	~ReadFileThread();
	void setFileName(QString name);
	void setFileNameList(QStringList nameList);
	void setFileFormat(int format);
	void setSolType(int type);
	void setKml(bool enable);
	void setBaseXYH(double x, double y, double hgt);
	void setUseAvg();
	void setDelayLimit(double delay);
	void setSecondOfMinute(int second);
	void setInterval(int interval);
	void setLoopTime(QString start, int loop, int interrupt);
	void setCenter(double center);
	void run();
	void stop();
protected:
	void init();
	void loadFile(QString fileName);
	void fillWithGGA(NEUCSV& neudata, ins_sol_t* ins, nmea_gga_t* gga);
	void utc2cst(char* cst_time, char* utc_time);
	void parseLine(char* line);
	void decode();
	void writeSingleFixFile();
	void writeMultiFixFile();
private:
	bool m_isStop;
	int m_Format;
	QString m_ReadFileName;
	QStringList m_FileNameList;
	QElapsedTimer m_TimeCounter;
	ASCII_Decoder* m_ASCII_Decoder;
	NMEA_Decoder* m_NMEA_Decoder;
	SPAN_ASCII_Decoder* m_SPAN_ASCII_Decoder;
	//NEUCSV neuData;
	QVector<NEUCSV> neuList;
	QVector<int> fixedTimeList;
	QMap<QString,QVector<int>> allFixedTimeList;
	CCoordinate m_crd;
	bool m_UseAvg;
	BLHCoord m_baseBLH;
	xyhCoord m_baseXYH;
	int m_SolSatusType;//0:all,1:fixed
	bool m_outputKml;
	double m_delayLimit;
	int m_secondOfMinute;
	int m_interval;
	QString m_startUTCTime;
	int m_startSecond;
	int m_loopSecond;
	int m_breakSecond;
	int m_loopCount;
	double m_Center;
	int m_nGGACountAll;
	int m_nHPRCountAll;
	int m_nHPRCountUsed;
signals:
	void sgnProgress(int present, int msecs);
	void sgnFinished(QString filename);
};
