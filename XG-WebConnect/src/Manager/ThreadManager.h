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
/// 线程中获取的数据都放到DataCache中，
/// 再通知窗体从DataCache中取数据。
/// </summary>

class ThreadManager : public QObject
{
    Q_OBJECT
public:
    static ThreadManager& Instance();
    ~ThreadManager();
private:
    ThreadManager();   // 构造函数私有化
    ThreadManager(ThreadManager const&); // 禁止拷贝构造
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
