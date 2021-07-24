//MIT License

//Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "finalmq/connectionhub/ConnectionHub.h"
#include <algorithm>


namespace finalmq {

ConnectionHub::ConnectionHub()
    : m_protocolSessionContainer(std::make_unique<ProtocolSessionContainer>())
{
    assert(m_protocolSessionContainer);
}



void ConnectionHub::init(int cycleTime)
{
    m_protocolSessionContainer->init(nullptr, cycleTime);
}

int ConnectionHub::bind(const std::string& endpoint, const BindProperties& bindProperties)
{
    return m_protocolSessionContainer->bind(endpoint, this, bindProperties);
}

void ConnectionHub::unbind(const std::string& endpoint)
{
    m_protocolSessionContainer->unbind(endpoint);
}

IProtocolSessionPtr ConnectionHub::connect(const std::string& endpoint, const ConnectProperties& connectProperties)
{
    IProtocolSessionPtr session = m_protocolSessionContainer->connect(endpoint, this, connectProperties);
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


void ConnectionHub::run()
{
    return m_protocolSessionContainer->run();
}

void ConnectionHub::waitForTerminationOfPollerLoop()
{
    m_protocolSessionContainer->terminatePollerLoop();
}



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
void ConnectionHub::connected(const IProtocolSessionPtr& /*session*/)
{
}

void ConnectionHub::disconnected(const IProtocolSessionPtr& /*session*/)
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

void ConnectionHub::socketConnected(const IProtocolSessionPtr& /*session*/)
{

}

void ConnectionHub::socketDisconnected(const IProtocolSessionPtr& /*session*/)
{

}

}   // namespace finalmq
