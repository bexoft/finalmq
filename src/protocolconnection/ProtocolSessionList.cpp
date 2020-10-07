
#include "protocolconnection/ProtocolSessionList.h"



ProtocolSessionList::ProtocolSessionList()
{
}

ProtocolSessionList::~ProtocolSessionList()
{
}

std::int64_t ProtocolSessionList::addProtocolSession(IProtocolSessionPtr ProtocolSession)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::int64_t sessionId = m_nextSessionId++;
    m_connectionId2ProtocolSession[sessionId] = ProtocolSession;
    lock.unlock();
    return sessionId;
}

void ProtocolSessionList::removeProtocolSession(std::int64_t sessionId)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_connectionId2ProtocolSession.erase(sessionId);
    lock.unlock();
}

std::vector< IProtocolSessionPtr > ProtocolSessionList::getAllSessions() const
{
    std::vector< IProtocolSessionPtr > ProtocolSessions;
    std::unique_lock<std::mutex> lock(m_mutex);
    ProtocolSessions.reserve(m_connectionId2ProtocolSession.size());
    for (auto it = m_connectionId2ProtocolSession.begin(); it != m_connectionId2ProtocolSession.end(); ++it)
    {
        ProtocolSessions.push_back(it->second);
    }
    lock.unlock();
    return ProtocolSessions;
}

IProtocolSessionPtr ProtocolSessionList::getSession(std::int64_t sessionId) const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_connectionId2ProtocolSession.find(sessionId);
    if (it != m_connectionId2ProtocolSession.end())
    {
        return it->second;
    }
    return nullptr;
}

