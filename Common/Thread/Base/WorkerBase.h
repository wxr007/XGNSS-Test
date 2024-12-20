#pragma once

#include <QObject>
#include <QFile>
#include <QThread>
#include "DataParser.h"

class WorkerBase  : public QObject
{
	Q_OBJECT

public:
	explicit WorkerBase(QObject* parent = nullptr, DataParser* dataparser = nullptr);
    virtual ~WorkerBase();
    void setLogFile(bool enable);
    void setFileName(QString sName);
public slots:
    virtual void onOpen();
    virtual void onClose();
    virtual void onWrite(const QByteArray data);
protected:
	void openDataFile();
	void closeDataFile();
	void writeDataLog(const QByteArray& data);
protected:
    bool m_isLogFile;
    QString m_FileNameFlag;
    QFile m_DataFile;
    DataParser* m_DataParser;
signals:
    void sgnStop();
    void sgnOpenFailed();
};

class WorkerThread : public QObject
{
    Q_OBJECT
public:
    explicit WorkerThread(QObject* parent = nullptr, WorkerBase* worker = nullptr);
    virtual ~WorkerThread();
    void setLogFile(bool enable);
    void setFileName(QString sName);
    void start();
    void stop();
    bool isRunning();
public slots:
    void onTerminate();
private:
    QThread* m_Thread;
protected:
    WorkerBase* m_Worker;
signals:
    void sgnOpen();
    void sgnClose();
    void sgnWrite(const QByteArray data);
    void sgnEnable(bool enbale);
    void sgnOpenFailed();
};