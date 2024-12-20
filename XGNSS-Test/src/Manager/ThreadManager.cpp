#include "ThreadManager.h"

std::once_flag ThreadManager::once_flag;
std::unique_ptr<ThreadManager> ThreadManager::instance;

ThreadManager& ThreadManager::Instance() {
    std::call_once(once_flag, [&]() {
        instance.reset(new ThreadManager());
    });
    return *instance;
}
ThreadManager::ThreadManager()
    : QObject(NULL)
{
    m_DataParser = new DataParser(NULL);
    m_ReplayThread = new ReplayThread(NULL);
    m_SerialThread = new SerialThread(NULL, m_DataParser);
    m_TcpClientThread = new TcpClientThread(NULL, m_DataParser);
    m_NtripThread = new NtripThread(NULL);
    m_RtkServerThread = new RtkServerThread(NULL);
    m_ReadFileThread = new ReadFileThread(NULL);

    connect(m_NtripThread, SIGNAL(sgnStream(const QByteArray)), m_SerialThread, SIGNAL(sgnWrite(const QByteArray)),Qt::QueuedConnection);
    connect(m_NtripThread, SIGNAL(sgnStream(const QByteArray)), m_TcpClientThread, SIGNAL(sgnWrite(const QByteArray)),Qt::QueuedConnection);
    //connect(m_RtkServerThread, SIGNAL(sgnStream(const QByteArray)), m_SerialThread, SIGNAL(sgnWrite(const QByteArray)), Qt::QueuedConnection);
    //connect(m_RtkServerThread, SIGNAL(sgnStream(const QByteArray)), m_TcpClientThread, SIGNAL(sgnWrite(const QByteArray)), Qt::QueuedConnection);
    connect(m_DataParser, SIGNAL(sgnNMEA(const QByteArray)), m_NtripThread, SIGNAL(sgnWrite(const QByteArray)), Qt::QueuedConnection);
}
ThreadManager::~ThreadManager()
{
    if (m_ReplayThread->isRunning()) {
        m_ReplayThread->stop();
        m_ReplayThread->wait();
        m_ReplayThread->closeFile();
    }
    m_SerialThread->stop();
    m_TcpClientThread->stop();
    m_NtripThread->stop();
    m_RtkServerThread->stop();
    m_ReadFileThread->stop();
}