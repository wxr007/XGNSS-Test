#include "DataParser.h"
#include "DataCache.h"
#include "parse_string.h"

DataParser::DataParser(QObject *parent)
	: QObject(parent)
	, isReadNmea(false)
{
	m_DataFormat = SPAN_Bin_Format;
	m_SPAN_ASCII_Decoder = new SPAN_ASCII_Decoder();
	m_SPAN_Binary_Decoder = new SPAN_Binary_Decoder();
	m_Hemisphere_Decoder = new Hemisphere_Decoder();
	m_NMEA_Decoder = new NMEA_Decoder();
	m_Spectrum_decoder = new Spectrum_decoder();
	m_SX7_Decoder = new SX7_Decoder();
}

DataParser::~DataParser()
{}

void DataParser::init()
{
	m_SPAN_ASCII_Decoder->init();
	m_SPAN_Binary_Decoder->init();
	m_Hemisphere_Decoder->init();
	m_NMEA_Decoder->init();
	m_Spectrum_decoder->init();
	m_SX7_Decoder->init();
	DataCache::Instance().m_update_map_offset = 1;
	DataCache::Instance().init();
	isReadNmea = false;
	m_readNMEACache.clear();	
}

void DataParser::setDataFormat(int index)
{
	m_DataFormat = index;
}

void DataParser::parseData(QByteArray& array)
{
	if (SPAN_Bin_Format == m_DataFormat) {
		processSPAN(array);
	}
	else if (Hemisphere_Format == m_DataFormat) {
		processHemisphere(array);
	}
	else if (NMEA_Format == m_DataFormat) {
		processNMEA(array);
	}
	else if (Spectrum_Format == m_DataFormat) {
		processSpectrum(array);
	}
	else if (SX7_Format == m_DataFormat) {
		processSX7(array);
	}

	if (Spectrum_Format != m_DataFormat) {
		showNMEA(array);
	}
}

void DataParser::setCalLineNum(int num)
{
	m_Spectrum_decoder->setCalLineNum(num);
}

void DataParser::processSPAN(QByteArray array)
{
	for (int i = 0; i < array.size(); i++) {
		int ret = m_SPAN_Binary_Decoder->input_raw(array[i]);
		if (ret > 0) {
			if (bestgnssposb == ret) {
				DataCache::Instance().setSpanGnssPos(m_SPAN_Binary_Decoder->get_header(), m_SPAN_Binary_Decoder->get_bestgnsspos());
				emit sgnUpdate(SolPosGnssType);
			}
			else if (bestgnssvelb == ret) {
				DataCache::Instance().setSpanGnssVel(m_SPAN_Binary_Decoder->get_header(), m_SPAN_Binary_Decoder->get_bestgnssvel());
				emit sgnUpdate(SolPosGnssType);

			}
			else if (inspvaxb == ret) {
				DataCache::Instance().setSpanIns(m_SPAN_Binary_Decoder->get_header(), m_SPAN_Binary_Decoder->get_inspvax());
				emit sgnUpdate(SolPosInsType);
			}
			else if (rawimu == ret || rawimusxb == ret || rawimusxbf == ret) {
				DataCache::Instance().setSpanImu(m_SPAN_Binary_Decoder->get_imu());
				emit sgnUpdate(SolImuType);
			}
			else if (2 == ret) {//nmea
				//NMEA_Decoder* nmea_decoder = m_SPAN_Binary_Decoder->get_nmea_decoder();
				//dispatchNMEASol(nmea_decoder, m_SPAN_Binary_Decoder->get_nmea_type());
				ret = m_NMEA_Decoder->input_txt_line(m_SPAN_Binary_Decoder->m_NmeaMsg);
				dispatchNMEASol(m_NMEA_Decoder, ret);
			}
		}
	}
}

void DataParser::processHemisphere(QByteArray array)
{
	for (int i = 0; i < array.size(); i++) {
		int ret = m_Hemisphere_Decoder->input_raw(array[i]);
		if (ret > 0) {
			if (eMsg3 == ret) {
				DataCache::Instance().setHemisphereMsg3(m_Hemisphere_Decoder->getSBinaryMsg3());
				emit sgnUpdateHemisphereMsg(eMsg3);
				DataCache::Instance().setHemisphereGnss(m_Hemisphere_Decoder->getSBinaryMsg3());
				emit sgnUpdate(SolPosGnssType);
			}
			else if (eMsg112 == ret) {
				DataCache::Instance().setHemisphereMsg112(m_Hemisphere_Decoder->getSBinaryMsg112());
				emit sgnUpdateHemisphereMsg(eMsg112);
				DataCache::Instance().setHemisphereIns(m_Hemisphere_Decoder->getSBinaryMsg112());
				emit sgnUpdate(SolPosInsType);
			}
			else if (eMsg111 == ret) {
				DataCache::Instance().setHemisphereMsg111(m_Hemisphere_Decoder->getSBinaryMsg111());
				emit sgnUpdateHemisphereMsg(eMsg111);
			}
			else if (eMsg309 == ret) {
				DataCache::Instance().m_mtx.lock();
				DataCache::Instance().setHemisphereMsg309(m_Hemisphere_Decoder->getMsg309());
				DataCache::Instance().m_mtx.unlock();
				emit sgnUpdateHemisphereMsg(eMsg309);
			}
			else if (eMsgNmea == ret) {
				ret = m_NMEA_Decoder->input_txt_line(m_Hemisphere_Decoder->m_NmeaMsg);
				dispatchNMEASol(m_NMEA_Decoder, ret);
			}
		}
	}
}

void DataParser::processNMEA(QByteArray array)
{
	int ret = m_NMEA_Decoder->input_txt_line(array.data());
	dispatchNMEASol(m_NMEA_Decoder, ret);
}

void DataParser::processSpectrum(QByteArray array)
{
	for (int i = 0; i < array.size(); i++) {
		int ret = m_Spectrum_decoder->input_raw(array[i]);
		if (ret == 1) {
			DataCache::Instance().setSpectrum(m_Spectrum_decoder->current_channel, m_Spectrum_decoder->y_val);
			emit sgnUpdateSpectrum(SpectrumType, m_Spectrum_decoder->current_channel);
		}
	}
}

void DataParser::processSX7(QByteArray array)
{
	for (int i = 0; i < array.size(); i++) {
		int ret = m_SX7_Decoder->input_raw(array[i]);
		if (ret == 1) {
			DataCache::Instance().m_SX7_time = m_SX7_Decoder->m_time;
			DataCache::Instance().m_SX7_raw.swap(m_SX7_Decoder->m_raw);
			m_SX7_Decoder->clear_list();
			emit sgnUpdateSX7();
		}
		else if (ret == 2) {//nmea
			ret = m_NMEA_Decoder->input_txt_line(m_SX7_Decoder->m_NmeaMsg);
			dispatchNMEASol(m_NMEA_Decoder, ret);
		}
	}
}

void DataParser::showNMEA(QByteArray& byteArray)
{
	//¹ýÂËnmeaÊý¾Ý
	for (int i = 0; i < byteArray.size(); i++) {
		if (!is_visable_char((char)byteArray[i])) {
			m_readNMEACache.clear();
			isReadNmea = false;
			continue;
		}
		if ((char)byteArray[i] == '$') {
			m_readNMEACache.clear();
			isReadNmea = true;
		}
		if (isReadNmea) {
			m_readNMEACache.push_back(byteArray[i]);
			if (m_readNMEACache.size() > 4 && (m_readNMEACache.endsWith("\r\n") || m_readNMEACache.endsWith("\n"))) {
				isReadNmea = false;
				/*m_dataCache.push_back(m_tempCache);*/
				emit sgnNMEA(m_readNMEACache);
				m_readNMEACache.clear();
			}
		}
	}
}

void DataParser::dispatchNMEASol(NMEA_Decoder* NMEA_Decoder, int ret)
{
	if ($GNGGA == ret) {
		DataCache::Instance().setNMEAGGA(NMEA_Decoder->getNmeaGGA());
		emit sgnUpdateNMEA($GNGGA);
	}
	else if ($GNHPR == ret) {
		NMEA_Decoder->getNmeaHPR();
		emit sgnUpdateNMEA($GNHPR);
	}
	else if ($GPGST == ret) {
		DataCache::Instance().setNMEAGST(NMEA_Decoder->getNmeaGST());
		emit sgnUpdateNMEA($GPGST);
	}
	else if ($GPVTG == ret) {
		DataCache::Instance().setNMEAVTG(NMEA_Decoder->getNmeaVTG());
		emit sgnUpdateNMEA($GPVTG);
	}
	else if ($GNHDT == ret) {
		DataCache::Instance().setNMEAHDT(NMEA_Decoder->getNmeaHDT());
		emit sgnUpdateNMEA($GNHDT);
	}
	else if (ret >= $GPGSV && ret <= $GQGSV) {//obs
		switch (ret)
		{
		case $GPGSV: {
			DataCache::Instance().m_mtx.lock();
			NMEA_Decoder->m_gps_obs.swap(DataCache::Instance().m_gps_obs);
			DataCache::Instance().m_mtx.unlock();
			NMEA_Decoder->m_gps_obs.clear();
			emit sgnUpdateGnss(ret);
		}break;
		case $GLGSV: {
			DataCache::Instance().m_mtx.lock();
			NMEA_Decoder->m_glo_obs.swap(DataCache::Instance().m_glo_obs);
			DataCache::Instance().m_mtx.unlock();
			NMEA_Decoder->m_glo_obs.clear();
			emit sgnUpdateGnss(ret);
		}break;
		case $GBGSV: {
			DataCache::Instance().m_mtx.lock();
			NMEA_Decoder->m_bds_obs.swap(DataCache::Instance().m_bds_obs);
			DataCache::Instance().m_mtx.unlock();
			NMEA_Decoder->m_bds_obs.clear();
			emit sgnUpdateGnss(ret);
		}break;
		case $GAGSV: {
			DataCache::Instance().m_mtx.lock();
			NMEA_Decoder->m_gal_obs.swap(DataCache::Instance().m_gal_obs);
			DataCache::Instance().m_mtx.unlock();
			NMEA_Decoder->m_gal_obs.clear();
			emit sgnUpdateGnss(ret);
		}break;
		case $GQGSV: {
			DataCache::Instance().m_mtx.lock();
			NMEA_Decoder->m_qzs_obs.swap(DataCache::Instance().m_qzs_obs);
			DataCache::Instance().m_mtx.unlock();
			NMEA_Decoder->m_qzs_obs.clear();
			emit sgnUpdateGnss(ret);
		}break;
		default:
			break;
		}
	}
}

void DataParser::onSetAntenna(int antenna)
{
	m_Hemisphere_Decoder->setAntenna(antenna);
}

void DataParser::onSetSpectrumWindowNum(int num)
{
	setCalLineNum(num);
}
