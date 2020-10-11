
#include "connectionhub/ConnectionHub.h"
#include <algorithm>



ConnectionHub::ConnectionHub()
    : m_protocolSessionContainer(std::make_unique<ProtocolSessionContainer>())
{
    assert(m_protocolSessionContainer);
}



void ConnectionHub::init(int cycleTime, int checkReconnectInterval)
{
    m_protocolSessionContainer->init(cycleTime, checkReconnectInterval);
}

int ConnectionHub::bind(const std::string& endpoint, IProtocolFactoryPtr protocolFactory)
{
    return m_protocolSessionContainer->bind(endpoint, this, protocolFactory);
}

void ConnectionHub::unbind(const std::string& endpoint)
{
    m_protocolSessionContainer->unbind(endpoint);
}

IProtocolSessionPtr ConnectionHub::connect(const std::string& endpoint, const IProtocolPtr& protocol, int reconnectInterval, int totalReconnectDuration)
{
    IProtocolSessionPtr session = m_protocolSessionContainer->connect(endpoint, this, protocol, reconnectInterval, totalReconnectDuration);
    return session;
}

std::vector< IProtocolSessionPtr > ConnectionHub::getAllSessions() const
{
    return m_protocolSessionContainer->getAllSessions();
}

IProtocolSessionPtr ConnectionHub::getSession(std::int64_t sessionId) const
{
    return m_protocolSessionContainer->getSession(sessionId);
}


void ConnectionHub::threadEntry()
{
    return m_protocolSessionContainer->threadEntry();
}

bool ConnectionHub::waitForTerminationOfPollerLoop(int timeout)
{
    return m_protocolSessionContainer->terminatePollerLoop(timeout);
}


#ifdef USE_OPENSSL
int ConnectionHub::bindSsl(const std::string& endpoint, IProtocolFactoryPtr protocolFactory, const CertificateData& certificateData)
{
    return m_protocolSessionContainer->bindSsl(endpoint, this, protocolFactory, certificateData);
}

IProtocolSessionPtr ConnectionHub::connectSsl(const std::string& endpoint, const IProtocolPtr& protocol, const CertificateData& certificateData, int reconnectInterval, int totalReconnectDuration)
{
    return m_protocolSessionContainer->connectSsl(endpoint, this, protocol, certificateData, reconnectInterval, totalReconnectDuration);
}

#endif


void ConnectionHub::startMessageForwarding()
{
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    m_startMessageForwarding = true;
    for (size_t i = 0; i < m_messagesForForwarding.size(); ++i)
    {
        auto& entry = m_messagesForForwarding[i];
        assert(entry.first);
        assert(entry.second);
        received(entry.first, entry.second);
    }
    m_messagesForForwarding.clear();
    lock.unlock();
}


void ConnectionHub::stopForwardingFromSession(std::int64_t sessionId)
{
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    m_sessionIdsStopForwardingFromSession.push_back(sessionId);
}

void ConnectionHub::stopForwardingToSession(std::int64_t sessionId)
{
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    m_sessionIdsStopForwardingToSession.push_back(sessionId);
}



// IProtocolSessionCallback
void ConnectionHub::connected(const IProtocolSessionPtr& session)
{
}

void ConnectionHub::disconnected(const IProtocolSessionPtr& session)
{

}


static bool contains(const std::vector<std::int64_t>& array, std::int64_t sessionId)
{
    if (std::find(array.begin(), array.end(), sessionId) != array.end())
    {
        return true;
    }
    return false;
}


void ConnectionHub::received(const IProtocolSessionPtr& session, const IMessagePtr& message)
{
    std::unique_lock<std::recursive_mutex> lock(m_mutex);

    if (contains(m_sessionIdsStopForwardingFromSession, session->getSessionId()))
    {
        return;
    }
    std::vector<std::int64_t> sessionIdsStopForwardingToSession = m_sessionIdsStopForwardingToSession;

    if (m_startMessageForwarding)
    {
        std::vector< IProtocolSessionPtr > sessions = m_protocolSessionContainer->getAllSessions();
        for (size_t i = 0; i < sessions.size(); ++i)
        {
            IProtocolSessionPtr s = sessions[i];
            assert(s);
            // if not from-session
            if (s != session)
            {
                // if not in sessionIdsStopForwardingToSession
                if (!contains(sessionIdsStopForwardingToSession, s->getSessionId()))
                {
                    s->sendMessage(message);
                }
            }
        }
    }
    else
    {
        m_messagesForForwarding.emplace_back(std::make_pair(session, message));
    }

    lock.unlock();
}

void ConnectionHub::socketConnected(const IProtocolSessionPtr& session)
{

}

void ConnectionHub::socketDisconnected(const IProtocolSessionPtr& session)
{

}
