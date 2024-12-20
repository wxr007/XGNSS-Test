#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_XGNSSTest.h"
#include "ads/API.h"
#include "ads/ContainerWidget.h"
#include "ads/SectionContent.h"

class ConfigFile;
class AboutDialog;

class XGNSSTest : public QMainWindow
{
    Q_OBJECT

public:
    XGNSSTest(QWidget *parent = nullptr, QString name = "", QString version = "");
    ~XGNSSTest();
protected:
    void defaultLayout();
    virtual void closeEvent(QCloseEvent* e);
    ADS_NS::SectionContent::RefPtr wrapWidget(QWidget* w, QString name);
    void SaveEvent();
    void LoadEvent();
    void saveConfig(QString filename);
    void loadConfig(QString filename);
    void ui2Json();
    void json2ui();
public slots:
    void onSetSimpleLayout();
    void onSetDefautLayout();
    void onShowSerialport();
    void onShowTcpClient();
    //void onShowRtkPort();
    void onShowNtripClient();
    void onShowCommandUI();
    void onShowDataTable();
    void onShowBinMsgTable();
    void onShowStarTable();
    void onShowLogFilePath();
    void onShowGGA2ZYZ();
    void onAboutDialog();
    void onActiveTabChanged(const ADS_NS::SectionContent::RefPtr& sc, bool active);
    void onSectionContentVisibilityChanged(const ADS_NS::SectionContent::RefPtr& sc, bool visible);
private:
    Ui::XGNSSTestClass ui;
    ConfigFile* m_pConfigFile;
    ADS_NS::ContainerWidget* _container;
    ADS_NS::SectionWidget* _sw;
    ADS_NS::SectionContent::RefPtr mSerialportUIPtr;
    ADS_NS::SectionContent::RefPtr mTcpClientUIPtr;
    //ADS_NS::SectionContent::RefPtr mRtkPortUIPtr;
    ADS_NS::SectionContent::RefPtr mNtripClientPtr;    
    ADS_NS::SectionContent::RefPtr mCommandUIPtr;
    ADS_NS::SectionContent::RefPtr mDataTableUIPtr;
    ADS_NS::SectionContent::RefPtr mBinMsgTableUIPtr;
    ADS_NS::SectionContent::RefPtr mStarTableUIPtr;
    ADS_NS::SectionContent::RefPtr mLogFilePathUIPtr;
    ADS_NS::SectionContent::RefPtr mTriggerCommandPtr;
    ADS_NS::SectionContent::RefPtr mHVChartPtr;
    ADS_NS::SectionContent::RefPtr mGGA2ZYZPtr;
    AboutDialog* m_AboutDialog;
    QByteArray mWindowGeometry;
    QByteArray mWindowLayout;
};
