#pragma once

#include <QObject>
#include "SPAN\SPAN_ASCII_Decoder.h"
#include "SPAN\SPAN_Binary_Decoder.h"
#include "Hemisphere\Hemisphere_Decoder.h"
#include "NMEA\NMEA_Decoder.h"
#include "Spectrum\Spectrum_decoder.h"
#include "SX7\SX7_Decoder.h"

class DataParser  : public QObject
{
	Q_OBJECT

public:
	DataParser(QObject *parent);
	~DataParser();
	void init();
	void setDataFormat(int index);
	void parseData(QByteArray& array);
	void setCalLineNum(int num);
protected:
	void processSPAN(QByteArray array);
	void processHemisphere(QByteArray array);
	void processNMEA(QByteArray array);
	void processSpectrum(QByteArray array);
	void processSX7(QByteArray array);
	void showNMEA(QByteArray& byteArray);
	void dispatchNMEASol(NMEA_Decoder* NMEA_Decoder,int ret);
protected slots:
	void onSetSpectrumWindowNum(int num);
	void onSetAntenna(int antenna);
private:
	int m_DataFormat;
	SPAN_ASCII_Decoder* m_SPAN_ASCII_Decoder;
	SPAN_Binary_Decoder* m_SPAN_Binary_Decoder;
	Hemisphere_Decoder* m_Hemisphere_Decoder;
	NMEA_Decoder* m_NMEA_Decoder;
	Spectrum_decoder* m_Spectrum_decoder;
	SX7_Decoder* m_SX7_Decoder;
	bool isReadNmea;
	QByteArray m_readNMEACache;
signals:
	void sgnUpdate(int type);
	void sgnUpdateSpectrum(int type, int channel);
	void sgnNMEA(const QByteArray data);
	void sgnUpdateGnss(int type);
	void sgnUpdateSX7();
	void sgnUpdateHemisphereMsg(int type);
	void sgnUpdateNMEA(int type);
};
