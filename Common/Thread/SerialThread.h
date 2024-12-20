#pragma once

#include <QObject>
#include "qextserialport.h"
#include "Base/WorkerBase.h"
#include "DataParser.h"

class SerialWorker : public WorkerBase
{
    Q_OBJECT
public:
    explicit SerialWorker(QObject* parent = nullptr, DataParser* dataparser = nullptr);
    ~SerialWorker();
    void setSerialPortParams(QString sName, int nBaudRate, int nDataBits=8, int nParity=0, int nStopBits=0);
    void setDataFormat(int index);
public slots:
    virtual void onOpen();
    virtual void onClose();
    virtual void onWrite(const QByteArray data);
protected slots:
    void onReadReady();
private:
    QextSerialPort* m_QextSerialPort;
signals:
    void sgnDataSize(const int);
};

class SerialThread : public WorkerThread
{
    Q_OBJECT
public:
    SerialThread(QObject* parent = nullptr, DataParser* dataparser = nullptr);
    ~SerialThread();
    void setSerialPortParams(QString sName, int nBaudRate, int nDataBits = 8, int nParity = 0, int nStopBits = 0);
    void setDataFormat(int index);
signals:
    void sgnDataSize(const int);
};