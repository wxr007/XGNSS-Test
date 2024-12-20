#pragma once

#include <mutex>
#include "SerialThread.h"
#include "TcpClientThread.h"
#include "NtripThread.h"
#include "RtkServerThread.h"
#include "WebSocketServerThread.h"
#include "DataParser.h"
#include <QObject>

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
    SerialThread* m_SerialThread;
    TcpClientThread* m_TcpClientThread;
    NtripThread* m_NtripThread;
    RtkServerThread* m_RtkServerThread;
    WebSocketServerThread* m_WebSocketServerThread;
};
