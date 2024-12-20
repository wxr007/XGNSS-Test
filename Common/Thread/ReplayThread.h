#pragma once

#include <QThread>
//#include <QTime>
#include <QElapsedTimer>
#include "SPAN\SPAN_Binary_Decoder.h"
#include "Hemisphere\Hemisphere_Decoder.h"
#include "Spectrum\Spectrum_decoder.h"

class ReplayThread  : public QThread
{
	Q_OBJECT

public:
	ReplayThread(QObject *parent);
	~ReplayThread();
	void run();
	void stop();
	void openFile(QString file_name);
	void closeFile();
	void setStartTime(double time);
	void setSpeed(int speed);
	void setInterval(int interval);
	void setDataFormat(int index);
protected:
	void replay();
	void replay_SPAN();
	void replay_hemisphere();
	void replay_Spectrum();
public slots:
	void onSetSpectrumWindowNum(int num);
signals:
	void sgnUpdate(int type);
	void sgnUpdateSpectrum(int type, int channel);
	void sgnProgress(int present, int msecs);
	void sgnFinish();
private:
	bool m_isStop;
	SPAN_Binary_Decoder* m_SPAN_Binary_Decoder;
	Hemisphere_Decoder* m_Hemisphere_Decoder;
	Spectrum_decoder* m_Spectrum_decoder;
	FILE* m_DataFile;
	double m_StartTime;
	double m_LastTime;
	QElapsedTimer m_TimeCounter;
	int m_Speed;
	int m_interval;
	int m_DataFormat;
};
