#pragma once

#include <mutex>
#include "Hemisphere\Hemisphere_Define.h"
#include "Format_Definition.h"
#include "Spectrum/Spectrum_decoder.h"
#include "SX7/SX7_Decoder.h"
#include "ReadFileThread.h"
#include <map>

enum emSolPosType {
    SolPosGnssType,
    SolPosInsType,
    SolImuType,
    SpectrumType,
};

enum emDataFormat {
    SPAN_Bin_Format,
    Hemisphere_Format,
    NMEA_Format,
    Spectrum_Format,
    SX7_Format,
};

struct ins_show_t {
    double       m_dGPSTimeOfWeek;         //GPS tow                              
    uint16_t     m_wGPSWeek;               //GPS week     
    uint16_t     m_wNavMode;               //navgiation mode    
    uint16_t     m_wStatus;                //navgiation status      
    double       m_dlatitude;              //unit: deg       
    double       m_dlongitude;             //unit: deg       
    float        m_fHeight;                //unit: m
    float        m_fSpeed;                 //unit: m/s
    float        m_fVelocityNorth;         //unit: m/s
    float        m_fVelocityEast;          //unit: m/s
    float        m_fVelocityUp;            //unit: m/s
    float        m_fRoll;                  //unit: deg
    float        m_fPitch;                 //unit: deg
    float        m_fYaw;                   //unit: deg

    float        m_fLat_std;                 //unit: m       
    float        m_fLon_std;                 //unit: m       
    float        m_fHgt_std;                 //unit: m
    float        m_fNV_std;                  //unit: m/s       
    float        m_fEV_std;                  //unit: m/s       
    float        m_fUV_std;                  //unit: m/s
    float        m_fRoll_std;                //unit: deg
    float        m_fPitch_std;               //unit: deg
    float        m_fYaw_std;                 //unit: deg
};

struct gnss_show_t {
    double      m_dGPSTimeOfWeek;           //GPS tow                              
    uint16_t    m_wGPSWeek;                 //GPS week    
    uint16_t    m_nPosType;                 //navgiation mode   
    double      m_dlatitude;                //unit: deg       
    double      m_dlongitude;               //unit: deg       
    float       m_fHeight;                  //unit: m
    float       m_fSpeed;                   //unit: m/s
    float       m_fVelocityNorth;           //unit: m/s
    float       m_fVelocityEast;            //unit: m/s
    float       m_fVelocityUp;              //unit: m/s
    float       m_fHeading;                 //Heading (degrees), Zero unless vector[4 bytes]     
    float       m_fPitch;                   //Pitch (degrees), Zero unless vector  [4 bytes] 
    float       m_fRoll;                    //Roll (degrees), Zero unless vector   [4 bytes]

    float       m_fLat_std;                 //unit: m       
    float       m_fLon_std;                 //unit: m       
    float       m_fHgt_std;                 //unit: m
    uint8_t     m_SVs;
    uint8_t     m_solSVs;
    float       m_fDiffAge;                 // age of differential, seconds         [2 bytes]
    float       m_fSolAge;
};

struct imu_show_t {
    uint16_t week;
    double seconds;
    float z_accel;
    float y_accel;
    float x_accel;
    float z_gyro;
    float y_gyro;
    float x_gyro;
};

struct GGA2ZYZCache {
    BLHCoord baseBLH;
    xyhCoord baseXYH;
    double minX;
    double maxX;
    double minY;
    double maxY;
    QList<QPointF> signalList;
    QList<QPointF> dGPSList;
    QList<QPointF> floatList;
    QList<QPointF> fixedList;
    QList<QPointF> insList;
    int signalCount;
    int dGPSCount;
    int floatCount;
    int fixedCount;
    int insCount;
    double minT;
    double maxT;
    double minH;
    double maxH;
    QList<QPointF> HValueList;
    QVector<int> fixedTimeList;
    GGA2ZYZCache() {
        init();
    }
    void init() {
        minX = DBL_MAX;
        maxX = DBL_MIN;
        minY = DBL_MAX;
        maxY = DBL_MIN;
        signalList.clear();
        dGPSList.clear();
        floatList.clear();
        fixedList.clear();
        insList.clear();
        signalCount = 0;
        dGPSCount = 0;
        floatCount = 0;
        fixedCount = 0;
        insCount = 0;
        minT = 0.0;
        maxT = 0.0;
        minH = DBL_MAX;
        maxH = DBL_MIN;
        HValueList.clear();
        fixedTimeList.clear();
    }
};

class DataCache
{
public:
    static DataCache& Instance();
private:
    DataCache();   // 构造函数私有化
    DataCache(DataCache const&); // 禁止拷贝构造
    DataCache& operator=(DataCache const&);

    static std::once_flag once_flag;
    static std::unique_ptr<DataCache> instance;
public:
    void init();
    void setHemisphereMsg3(SBinaryMsg3* msg);
    void setHemisphereMsg111(SBinaryMsg111* msg);
    void setHemisphereMsg112(SBinaryMsg112* msg);
    void setHemisphereMsg309(UnfoldMsg309* msg);
    void accumulateHemisphereMsg(int msgId);
    void setHemisphereGnss(SBinaryMsg3* ins);
    void setHemisphereIns(SBinaryMsg112* ins);
    void setNMEAGnss(ins_sol_t* ins);
    void setNMEAGGA(nmea_gga_t* gga);
    void setNMEAHPR(nmea_hpr_t* hpr);
    void setNMEAGST(nmea_gst_t* gst);
    void setNMEAVTG(nmea_vtg_t* vtg);
    void setNMEAHDT(nmea_hdt_t* htd);
    void setSpanGnssPos(span_header_t* header, bestgnsspos_t* gnss);
    void setSpanGnssVel(span_header_t* header, bestgnssvel_t* vel);
    void setSpanIns(span_header_t* header, inspvax_t* ins);
    void setSpanImu(rawimusxf_t* ins);
    void setSpectrum(int channel, double* Spectrum);
public:
    ins_show_t m_ins_show;
    gnss_show_t m_gnss_show;
    imu_show_t m_imu_show;
    SBinaryMsg3 sMsg003;
    SBinaryMsg111 sMsg111;
    SBinaryMsg112 sMsg112;
    UnfoldMsg309 unfoldMsg309;
    std:: map<int, int> mHemisphereMsgCount;
    int m_update_map_offset;
    double Spectrum_Y[3][Spectrum_MAX_VAL_COUNT];
    std::vector<sat_obs_t> m_gps_obs;
    std::vector<sat_obs_t> m_glo_obs;
    std::vector<sat_obs_t> m_bds_obs;
    std::vector<sat_obs_t> m_gal_obs;
    std::vector<sat_obs_t> m_qzs_obs;
    std::mutex m_mtx;
    int m_SX7_time;
    vector<SX7Raw_t> m_SX7_raw;
    nmea_gga_t m_nmea_gga;
    nmea_hpr_t m_nmea_hpr;
    nmea_gst_t m_nmea_gst;
    nmea_vtg_t m_nmea_vtg;
    nmea_hdt_t m_nmea_hdt;
    GGA2ZYZCache m_GGA2ZYZCache;
};
