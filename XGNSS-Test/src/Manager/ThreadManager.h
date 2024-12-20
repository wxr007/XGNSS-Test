#pragma once

#include <mutex>
#include "ReplayThread.h"
#include "SerialThread.h"
#include "TcpClientThread.h"
#include "NtripThread.h"
#include "RtkServerThread.h"
#include "ReadFileThread.h"
#include "DataParser.h"
#include <QObject>
#include <QMutex>
#include <QWaitCondition>

/// <summary>
/// �߳��л�ȡ�����ݶ��ŵ�DataCache�У�
/// ��֪ͨ�����DataCache��ȡ���ݡ�
/// </summary>

class ThreadManager : public QObject
{
    Q_OBJECT
public:
    static ThreadManager& Instance();
    ~ThreadManager();
private:
    ThreadManager();   // ���캯��˽�л�
    ThreadManager(ThreadManager const&); // ��ֹ��������
    ThreadManager& operator=(ThreadManager const&);
   
    static std::once_flag once_flag;
    static std::unique_ptr<ThreadManager> instance;
public:
    DataParser* m_DataParser;
    ReplayThread* m_ReplayThread;
    SerialThread* m_SerialThread;
    TcpClientThread* m_TcpClientThread;
    NtripThread* m_NtripThread;
    RtkServerThread* m_RtkServerThread;
    ReadFileThread* m_ReadFileThread;

    QMutex m_ReadFileMutex;
    QWaitCondition m_ReadFileCondition;
    bool m_ReadFileFlag;
};
