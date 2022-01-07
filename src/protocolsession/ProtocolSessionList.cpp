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


#include "finalmq/protocolsession/ProtocolSessionList.h"
#include "finalmq/protocolsession/ProtocolSession.h"


namespace finalmq {


std::atomic_int64_t ProtocolSessionList::m_nextSessionId{1};


ProtocolSessionList::ProtocolSessionList()
{
}

ProtocolSessionList::~ProtocolSessionList()
{
}

std::int64_t ProtocolSessionList::addProtocolSession(IProtocolSessionPrivatePtr ProtocolSession, bool verified)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::int64_t sessionId = m_nextSessionId.fetch_add(1);
    m_connectionId2ProtocolSession[sessionId] = { ProtocolSession, verified, {} };
    lock.unlock();
    return sessionId;
}

void ProtocolSessionList::removeProtocolSession(std::int64_t sessionId)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_connectionId2ProtocolSession.erase(sessionId);
    lock.unlock();
}

std::vector< IProtocolSessionPrivatePtr > ProtocolSessionList::getAllSessions() const
{
    std::vector< IProtocolSessionPrivatePtr > protocolSessions;
    std::unique_lock<std::mutex> lock(m_mutex);
    protocolSessions.reserve(m_connectionId2ProtocolSession.size());
    for (auto it = m_connectionId2ProtocolSession.begin(); it != m_connectionId2ProtocolSession.end(); ++it)
    {
        if (it->second.verified)
        {
            protocolSessions.emplace_back(it->second.session);
        }
    }
    lock.unlock();
    return protocolSessions;
}

IProtocolSessionPtr ProtocolSessionList::getSession(std::int64_t sessionId) const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_connectionId2ProtocolSession.find(sessionId);
    if (it != m_connectionId2ProtocolSession.end())
    {
        if (it->second.verified)
        {
            return it->second.session;
        }
    }
    return nullptr;
}


IProtocolSessionPrivatePtr ProtocolSessionList::findSessionByName(const std::string& sessionName) const
{
    IProtocolSessionPrivatePtr protocolSessions;
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_connectionId2ProtocolSession.begin(); it != m_connectionId2ProtocolSession.end(); ++it)
    {
        if (it->second.verified && it->second.name == sessionName)
        {
            return it->second.session;
        }
    }
    lock.unlock();
    return nullptr;
}


bool ProtocolSessionList::setSessionName(std::int64_t sessionId, const std::string& sessionName)
{
    bool ok = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_connectionId2ProtocolSession.find(sessionId);
    if (it != m_connectionId2ProtocolSession.end())
    {
        if (!it->second.verified)
        {
            ok = true;
            it->second.name = sessionName;
            it->second.verified = true;
        }
    }
    return ok;
}



}   // namespace finalmq
