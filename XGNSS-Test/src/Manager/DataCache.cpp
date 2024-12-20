#include "DataCache.h"
#include "gnss_math.h"
#include "Hemisphere\Hemisphere_Decoder.h"
#include "SPAN\SPAN_Binary_Decoder.h"

std::once_flag DataCache::once_flag;
std::unique_ptr<DataCache> DataCache::instance;

DataCache& DataCache::Instance() {
    std::call_once(once_flag, [&]() {
        instance.reset(new DataCache());
    });
    return *instance;
}
DataCache::DataCache()
{
    memset(&m_gnss_show, 0, sizeof(m_gnss_show));
    memset(&m_ins_show, 0, sizeof(m_ins_show));
    memset(&m_imu_show, 0, sizeof(m_imu_show));
    memset(&sMsg003, 0, sizeof(sMsg003));
    memset(&sMsg111, 0, sizeof(sMsg111));
    memset(&sMsg112, 0, sizeof(sMsg112));
    memset(&m_nmea_gga, 0, sizeof(m_nmea_gga));
    memset(&m_nmea_hpr, 0, sizeof(m_nmea_hpr));
    memset(&m_nmea_gst, 0, sizeof(m_nmea_gst));
    memset(&m_nmea_vtg, 0, sizeof(m_nmea_vtg));
    memset(&m_nmea_hdt, 0, sizeof(m_nmea_hdt));
    m_update_map_offset = 0;
    init();
}
void DataCache::init()
{
    mHemisphereMsgCount.clear();
    m_gps_obs.clear();
    m_glo_obs.clear();
    m_bds_obs.clear();
    m_gal_obs.clear();
    m_qzs_obs.clear();

    m_SX7_time = 0;
    m_SX7_raw.clear();
}
void DataCache::setHemisphereMsg3(SBinaryMsg3* msg)
{
    memcpy(&sMsg003, msg, sizeof(sMsg003));
    accumulateHemisphereMsg(eMsg3);
}
void DataCache::setHemisphereMsg111(SBinaryMsg111* msg)
{
    memcpy(&sMsg111, msg, sizeof(sMsg111));
    accumulateHemisphereMsg(eMsg111);
}
void DataCache::setHemisphereMsg112(SBinaryMsg112* msg)
{
    memcpy(&sMsg112, msg, sizeof(sMsg112));
    accumulateHemisphereMsg(eMsg112);
}
void DataCache::setHemisphereMsg309(UnfoldMsg309* msg)
{
    unfoldMsg309.msg309Sats.swap(msg->msg309Sats);
    unfoldMsg309.m_Antenna = msg->m_Antenna;
    unfoldMsg309.m_wGPSWeek = msg->m_wGPSWeek;
    unfoldMsg309.m_dGPSTimeOfWeek = msg->m_dGPSTimeOfWeek;
    accumulateHemisphereMsg(eMsg309);
}
void DataCache::accumulateHemisphereMsg(int msgId)
{
    if (mHemisphereMsgCount.find(msgId) != mHemisphereMsgCount.end()) {
        mHemisphereMsgCount[msgId]++;
    }
    else {
        mHemisphereMsgCount[msgId] = 1;
    }
}
void DataCache::setHemisphereGnss(SBinaryMsg3* gnss)
{
    float north_vel = gnss->m_fSpeed * cos(D2R(gnss->m_fCOG));
    float east_vel = gnss->m_fSpeed * sin(D2R(gnss->m_fCOG));

    m_gnss_show.m_wGPSWeek = gnss->m_wGPSWeek;
    m_gnss_show.m_dGPSTimeOfWeek = gnss->m_dGPSTimeOfWeek;
    m_gnss_show.m_nPosType = Hemisphere_Decoder::transformNavMode(gnss->m_byNavMode);
    m_gnss_show.m_dlatitude = gnss->m_dLatitude;
    m_gnss_show.m_dlongitude = gnss->m_dLongitude;
    m_gnss_show.m_fHeight = gnss->m_fHeight;
    m_gnss_show.m_fSpeed = gnss->m_fSpeed;
    m_gnss_show.m_fVelocityNorth = north_vel;
    m_gnss_show.m_fVelocityEast = east_vel;
    m_gnss_show.m_fVelocityUp = gnss->m_fVUp;
    m_gnss_show.m_fHeading = gnss->m_fHeading;
    m_gnss_show.m_fPitch = gnss->m_fPitch;
    m_gnss_show.m_fRoll = gnss->m_fRoll;

    //m_ins_show.m_wNavMode = gnss->m_byNavMode;
    //m_ins_show.m_dlatitude = gnss->m_dLatitude;
    //m_ins_show.m_dlongitude = gnss->m_dLongitude;
    //m_ins_show.m_fHeight = gnss->m_fHeight;
    //m_ins_show.m_fSpeed = gnss->m_fSpeed;
    //m_ins_show.m_fVelocityNorth = north_vel;
    //m_ins_show.m_fVelocityEast = east_vel;
    //m_ins_show.m_fVelocityDown = -gnss->m_fVUp;
}

void DataCache::setHemisphereIns(SBinaryMsg112* ins)
{
    m_ins_show.m_wGPSWeek = ins->m_wGPSWeek;
    m_ins_show.m_dGPSTimeOfWeek = ins->m_dGPSTimeOfWeek;
    m_ins_show.m_wStatus = ins->m_lStatus;
    m_ins_show.m_wNavMode = ins->m_wNavMode;
    m_ins_show.m_dlatitude = ins->m_dlatitude;
    m_ins_show.m_dlongitude = ins->m_dlongitude;
    m_ins_show.m_fHeight = ins->m_fHeight;
    m_ins_show.m_fVelocityNorth = ins->m_fVelocityNorth;
    m_ins_show.m_fVelocityEast = ins->m_fVelocityEast;
    m_ins_show.m_fVelocityUp = -ins->m_fVelocityDown;
    m_ins_show.m_fRoll = ins->m_fRoll;
    m_ins_show.m_fPitch = ins->m_fPitch;
    m_ins_show.m_fYaw = ins->m_fAzimuth;
    //std
    m_ins_show.m_fLat_std = ins->m_fPosStdNorth;
    m_ins_show.m_fLon_std = ins->m_fPosStdEast;
    m_ins_show.m_fHgt_std = -ins->m_fPosStdDown;
    m_ins_show.m_fNV_std = ins->m_fVelStdNorth;
    m_ins_show.m_fEV_std = ins->m_fVelStdEast;
    m_ins_show.m_fUV_std = -ins->m_fVelStdDown;
    m_ins_show.m_fRoll_std = ins->m_fRollStd;
    m_ins_show.m_fPitch_std = ins->m_fPitchStd;
    m_ins_show.m_fYaw_std = ins->m_fAzimuthStd;
}

void DataCache::setNMEAGnss(ins_sol_t* ins) {
    //if (m_gnss_show.m_wGPSWeek != 0) {
    //    return;
    //}
    m_gnss_show.m_wGPSWeek = ins->wn;
    m_gnss_show.m_dGPSTimeOfWeek = ins->ws;
    m_gnss_show.m_nPosType = ins->sol_type;
    m_gnss_show.m_dlatitude = ins->blh[0];
    m_gnss_show.m_dlongitude = ins->blh[1];
    m_gnss_show.m_fHeight = ins->blh[2];

    m_gnss_show.m_fSpeed = sqrt(ins->vel_NED[0]* ins->vel_NED[0] + ins->vel_NED[1] * ins->vel_NED[1]);
    m_gnss_show.m_fVelocityNorth = ins->vel_NED[0];
    m_gnss_show.m_fVelocityEast = ins->vel_NED[1];
    m_gnss_show.m_fVelocityUp = -ins->vel_NED[2];
}

void DataCache::setNMEAGGA(nmea_gga_t* gga) {
    memcpy(&m_nmea_gga, gga, sizeof(nmea_gga_t));
}

void DataCache::setNMEAHPR(nmea_hpr_t* hpr)
{
    memcpy(&m_nmea_hpr, hpr, sizeof(nmea_hpr_t));
}

void DataCache::setNMEAGST(nmea_gst_t* gst)
{
    memcpy(&m_nmea_gst, gst, sizeof(nmea_gst_t));
}

void DataCache::setNMEAVTG(nmea_vtg_t* vtg)
{
    memcpy(&m_nmea_vtg, vtg, sizeof(nmea_vtg_t));
}

void DataCache::setNMEAHDT(nmea_hdt_t* htd)
{
    memcpy(&m_nmea_hdt, htd, sizeof(nmea_hdt_t));
}

void DataCache::setSpanGnssPos(span_header_t* header,bestgnsspos_t* gnss)
{
    m_gnss_show.m_wGPSWeek = header->week;
    m_gnss_show.m_dGPSTimeOfWeek = header->gps_seconds;
    m_gnss_show.m_nPosType = SPAN_Binary_Decoder::get_pos_type(gnss->position_type);
    m_gnss_show.m_dlatitude = gnss->lat;
    m_gnss_show.m_dlongitude = gnss->lon;
    m_gnss_show.m_fHeight = gnss->hgt;

    m_gnss_show.m_fLat_std = gnss->lat_sigma;
    m_gnss_show.m_fLon_std = gnss->lon_sigma;
    m_gnss_show.m_fHgt_std = gnss->height_sigma;

    m_gnss_show.m_SVs = gnss->SVs;
    m_gnss_show.m_solSVs = gnss->solnSVs;

    m_gnss_show.m_fDiffAge = gnss->diff_age;
    m_gnss_show.m_fSolAge = gnss->sol_age;
}
void DataCache::setSpanGnssVel(span_header_t* header, bestgnssvel_t* vel)
{
    double north_velocity = vel->hor_spd * cos(vel->trk_gnd * PI / 180);
    double east_velocity = vel->hor_spd * sin(vel->trk_gnd * PI / 180);
    double up_velocity = vel->vert_spd;
    m_gnss_show.m_fVelocityNorth = north_velocity;
    m_gnss_show.m_fVelocityEast = east_velocity;
    m_gnss_show.m_fVelocityUp = up_velocity;
}

void DataCache::setSpanIns(span_header_t* header, inspvax_t* ins)
{
    m_ins_show.m_wGPSWeek = header->week;
    m_ins_show.m_dGPSTimeOfWeek = header->gps_seconds;
    m_ins_show.m_wStatus = ins->ins_status;
    m_ins_show.m_wNavMode = ins->position_type;
    m_ins_show.m_dlatitude = ins->latitude;
    m_ins_show.m_dlongitude = ins->longitude;
    m_ins_show.m_fHeight = ins->altitude + ins->undulation;
    m_ins_show.m_fVelocityNorth = ins->north_velocity;
    m_ins_show.m_fVelocityEast = ins->east_velocity;
    m_ins_show.m_fVelocityUp = ins->up_velocity;
    m_ins_show.m_fRoll = ins->roll;
    m_ins_show.m_fPitch = ins->pitch;
    m_ins_show.m_fYaw = ins->azimuth;
    m_ins_show.m_fSpeed = sqrt(ins->north_velocity * ins->north_velocity + ins->east_velocity * ins->east_velocity);
    //std
    m_ins_show.m_fLat_std = ins->latitude_std;
    m_ins_show.m_fLon_std = ins->longitude_std;
    m_ins_show.m_fHgt_std = ins->altitude_std;
    m_ins_show.m_fNV_std = ins->north_velocity_std;
    m_ins_show.m_fEV_std = ins->east_velocity_std;
    m_ins_show.m_fUV_std = ins->up_velocity_std;
    m_ins_show.m_fRoll_std = ins->roll_std;
    m_ins_show.m_fPitch_std = ins->pitch_std;
    m_ins_show.m_fYaw_std = ins->azimuth_std;
}

void DataCache::setSpanImu(rawimusxf_t* imu)
{
    m_imu_show.week = imu->week;
    m_imu_show.seconds = imu->seconds;
    m_imu_show.x_accel = imu->x_accel;
    m_imu_show.y_accel = imu->y_accel;
    m_imu_show.z_accel = imu->z_accel;
    m_imu_show.x_gyro = imu->x_gyro;
    m_imu_show.y_gyro = imu->y_gyro;
    m_imu_show.z_gyro = imu->z_gyro;
}

void DataCache::setSpectrum(int channel, double* Spectrum)
{
    int index = 0;
    if (channel == 0) {
        index = 0;
    }
    else if (channel == 2) {
        index = 1;
    }
    else if (channel == 4) {
        index = 2;
    }
    for (size_t i = 1; i < Spectrum_MAX_VAL_COUNT; i++) {
        Spectrum_Y[index][i] = Spectrum[i];
    }
    Spectrum_Y[index][0] = channel;

}