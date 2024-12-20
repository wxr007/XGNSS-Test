#include "ReplayThread.h"
#include "DataCache.h"
#include "file_info.h"

#define READ_CACHE_SIZE 4096

ReplayThread::ReplayThread(QObject *parent)
	: QThread(parent)
	, m_isStop(false)
	, m_StartTime(0.0)
	, m_LastTime(0.0)
	, m_Speed(0)
	, m_interval(10)
	, m_DataFormat(0)
{
	m_DataFile = NULL;
	m_SPAN_Binary_Decoder = new SPAN_Binary_Decoder();
	m_Hemisphere_Decoder = new Hemisphere_Decoder();
	m_Spectrum_decoder = new Spectrum_decoder();
}

ReplayThread::~ReplayThread()
{
	delete m_SPAN_Binary_Decoder;
	delete m_Hemisphere_Decoder;
	delete m_Spectrum_decoder;
}

void ReplayThread::run()
{
	m_isStop = false;
	m_TimeCounter.start();
	replay();
	emit sgnFinish();
}

void ReplayThread::stop()
{
	m_isStop = true;
}

void ReplayThread::openFile(QString file_name)
{
	m_DataFile = fopen(file_name.toLocal8Bit().data(), "rb");
}

void ReplayThread::closeFile()
{
	if (!m_isStop) {
		return;
	}
	if (m_DataFile) {
		fclose(m_DataFile);
		m_DataFile = NULL;
	}
}

void ReplayThread::setStartTime(double time)
{
	m_StartTime = time;
}

void ReplayThread::setSpeed(int speed)
{
	m_Speed = speed;
}

void ReplayThread::setInterval(int interval)
{
	m_interval = interval;
}

void ReplayThread::setDataFormat(int index)
{
	m_DataFormat = index;
}

void ReplayThread::onSetSpectrumWindowNum(int num)
{
	m_Spectrum_decoder->setCalLineNum(num);
}

void ReplayThread::replay() {
	if (m_DataFormat == SPAN_Bin_Format) {
		replay_SPAN();
	}
	else if (m_DataFormat == Hemisphere_Format) {
		replay_hemisphere();
	}
	else if (m_DataFormat == Spectrum_Format) {
		replay_Spectrum();
	}
}

void ReplayThread::replay_SPAN()
{
	if (m_DataFile && m_SPAN_Binary_Decoder) {
		size_t file_size = getFileSize(m_DataFile);
		size_t read_size = 0;
		size_t readcount = 0;
		char read_cache[READ_CACHE_SIZE] = { 0 };
		m_SPAN_Binary_Decoder->init();
		DataCache::Instance().m_update_map_offset = 1;//开始时候要更新一下地图的偏移
		while (!feof(m_DataFile)) {
			if (m_isStop) break;
			readcount = fread(read_cache, sizeof(char), READ_CACHE_SIZE, m_DataFile);
			read_size += readcount;
			for (int i = 0; i < readcount; i++) {
				int ret = m_SPAN_Binary_Decoder->input_raw(read_cache[i]);
				if (ret > 0) {
					m_LastTime = m_SPAN_Binary_Decoder->get_header()->gps_seconds;
					if (m_LastTime < m_StartTime) {
						continue;
					}
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
						QThread::msleep(m_interval);
					}
					else if (rawimu == ret || rawimusxb == ret || rawimusxbf == ret) {
						DataCache::Instance().setSpanImu(m_SPAN_Binary_Decoder->get_imu());
						emit sgnUpdate(SolImuType);
					}
				}
			}
			double percent = (double)read_size / (double)file_size * 10000;
			emit sgnProgress((int)percent, m_TimeCounter.elapsed());
		}
		m_SPAN_Binary_Decoder->finish();
	}
}

void ReplayThread::replay_hemisphere()
{
	if (m_DataFile && m_Hemisphere_Decoder) {
		size_t file_size = getFileSize(m_DataFile);
		size_t read_size = 0;
		size_t readcount = 0;
		char read_cache[READ_CACHE_SIZE] = { 0 };
		m_Hemisphere_Decoder->init();
		DataCache::Instance().m_update_map_offset = 1;//开始时候要更新一下地图的偏移
		while (!feof(m_DataFile)) {
			if (m_isStop) break;
			readcount = fread(read_cache, sizeof(char), READ_CACHE_SIZE, m_DataFile);
			read_size += readcount;
			for (int i = 0; i < readcount; i++) {
				int ret = m_Hemisphere_Decoder->input_raw(read_cache[i]);
				if (ret > 0) {
					if (ret == eMsg112) {
						m_LastTime = m_Hemisphere_Decoder->getSBinaryMsg112()->m_dGPSTimeOfWeek;
						DataCache::Instance().setHemisphereIns(m_Hemisphere_Decoder->getSBinaryMsg112());
						if (m_LastTime < m_StartTime) {
							continue;
						}
						emit sgnUpdate(SolPosInsType);
						QThread::msleep(50/m_Speed);
					}
					else if (ret == eMsg3) {
						m_LastTime = m_Hemisphere_Decoder->getSBinaryMsg3()->m_dGPSTimeOfWeek;
						DataCache::Instance().setHemisphereGnss(m_Hemisphere_Decoder->getSBinaryMsg3());
						if (m_LastTime < m_StartTime) {
							continue;
						}
						emit sgnUpdate(SolPosGnssType);
						//QThread::msleep(500);
					}
				}
			}
			double percent = (double)read_size / (double)file_size * 10000;
			emit sgnProgress((int)percent, m_TimeCounter.elapsed());
		}
		m_Hemisphere_Decoder->finish();
	}
}

void ReplayThread::replay_Spectrum()
{
	if (m_DataFile && m_Spectrum_decoder) {
		size_t file_size = getFileSize(m_DataFile);
		size_t read_size = 0;
		size_t readcount = 0;
		char read_cache[READ_CACHE_SIZE] = { 0 };
		m_Spectrum_decoder->init();
		while (!feof(m_DataFile)) {
			if (m_isStop) break;
			readcount = fread(read_cache, sizeof(char), READ_CACHE_SIZE, m_DataFile);
			read_size += readcount;
			for (int i = 0; i < readcount; i++) {
				int ret = m_Spectrum_decoder->input_raw(read_cache[i]);
				if (ret == 1) {
					DataCache::Instance().setSpectrum(m_Spectrum_decoder->current_channel, m_Spectrum_decoder->y_val);
					emit sgnUpdateSpectrum(SpectrumType, m_Spectrum_decoder->current_channel);
					QThread::sleep(1);
				}
			}
			double percent = (double)read_size / (double)file_size * 10000;
			emit sgnProgress((int)percent, m_TimeCounter.elapsed());
		}
	}
}

